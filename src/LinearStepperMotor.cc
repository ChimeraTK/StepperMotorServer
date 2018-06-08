#include "LinearStepperMotor.h"
#include <mtca4u/DMapFilesParser.h>
#include <mtca4u/MapFileParser.h>
#include <mtca4u/PcieBackend.h>
#include <mtca4u/DeviceInfoMap.h>
#include <iostream>
#include "StepperMotorException.h"

const int StepperMotor::MOTOR_OK = 1;
const int StepperMotor::MOTOR_DISABLED = 2;
const int StepperMotor::MOTOR_IN_MOVE = 4;
const int StepperMotor::MOTOR_NOT_IN_POSITION = 8;
const int StepperMotor::MOTOR_SOFT_POS_LIMIT_REACHED = 16;
const int StepperMotor::MOTOR_SOFT_NEG_LIMIT_REACHED = 32;
const int StepperMotor::MOTOR_ERROR = 64;
const int StepperMotor::MOTOR_HARD_POS_LIMIT_REACHED = 128;
const int StepperMotor::MOTOR_HARD_NEG_LIMIT_REACHED = 256;

const int StepperMotor::MOTOR_NO_ERROR = 1;
const int StepperMotor::MOTOR_COMMUNICATION_LOST = 2;
const int StepperMotor::MOTOR_CONFIG_ERROR = 4;
const int StepperMotor::MOTOR_NOT_RESPONDING = 8;
const int StepperMotor::MOTOR_HARDWARE_NOT_CONNECTED = 16;
const int StepperMotor::MOTOR_BOTH_END_SWITCH_ON = 32;


boost::shared_ptr<StepperMotor> StepperMotor::create(int bamMotorID,
                                                           std::string motorDriverCardDeviceName,
                                                           std::string moduleName,
                                                           unsigned int motorDriverId,
                                                           std::string motorDriverCardConfigFileName,
                                                           std::string motorServerAddress){
  // Rather configure motor type from script?!
  if (motorServerAddress != ""){
    return boost::shared_ptr<StepperMotor>(new BeckhoffLinearStepperMotor(bamMotorID,
                                                                             motorServerAddress));
  }else if (motorServerAddress == "" && motorDriverCardDeviceName == ""){
    return boost::shared_ptr<StepperMotor>(new StepperMotorImp(bamMotorID));
  }else{
    mtca4u::DeviceInfoMap::DeviceInfo deviceInfo
    = mtca4u::DMapFilesParser(".").getdMapFileElem("MOTOR_DEVICE");
    if (deviceInfo.deviceName == "/dummy/MotorDriverCard"){
      return boost::shared_ptr<StepperMotor>(new DummyLinearStepperMotor(bamMotorID,
                                                                            motorDriverCardDeviceName,
                                                                            moduleName,
                                                                            motorDriverId,
                                                                            motorDriverCardConfigFileName));
    }else{
      return boost::shared_ptr<StepperMotor>(new MD22LinearStepperMotor(bamMotorID,
                                                                           motorDriverCardDeviceName,
                                                                           moduleName,
                                                                           motorDriverId,
                                                                           motorDriverCardConfigFileName));
    }
  }
}

// FIXME BAM_RELATED
const double StepperMotor::mmPerFemtoseconds = 2.99706287837918871e-04;

StepperMotor::StepperMotor(int bamMotorID) :
    								    _motorID(bamMotorID),
    								    _maxPos(1e8),
    								    _minPos(-1e8),
    								    _unitsConverter(new MotorUnitsConverter()){}

void StepperMotor::setUnitsConverter(boost::shared_ptr<mtca4u::StepperMotorUnitsConverter> &unitsConverter){
  _unitsConverter.reset();
  _unitsConverter = unitsConverter;
}

void StepperMotor::setSoftLimitsMotorPosition(float maxPos, float minPos){
  int tempMax = _unitsConverter->unitsToSteps(maxPos);
  int tempMin = _unitsConverter->unitsToSteps(minPos);

  if (tempMax > tempMin){
    _maxPos = tempMax;
    _minPos = tempMin;
  }else{
    _maxPos = tempMin;
    _minPos = tempMax;
  }
}

void StepperMotor::getSoftLimitsMotorPosition(float &maxPos, float &minPos){
  maxPos = _maxPos;
  minPos = _minPos;
}

StepperMotor::~StepperMotor(){}

MD22LinearStepperMotor::MD22LinearStepperMotor(int bamMotorID,
                                               std::string motorDriverCardDeviceName,
                                               std::string moduleName,
                                               unsigned int motorDriverId,
                                               std::string motorDriverCardConfigFileName) :
                			           mtca4u::LinearStepperMotor (motorDriverCardDeviceName,
                			                                       moduleName,
                			                                       motorDriverId,
                			                                       motorDriverCardConfigFileName),
                			                                       StepperMotor (bamMotorID),
                			                                       _isCalibrated(false),
                			                                       _future(){
  this->setEnabled(true);
  setCurrentPositionAs(0);
  mtca4u::LinearStepperMotor::setMaxPositionLimit(_maxPos);
  mtca4u::LinearStepperMotor::setMinPositionLimit(_minPos);
}

MD22LinearStepperMotor::~MD22LinearStepperMotor(){}

bool MD22LinearStepperMotor::startMove(float newPos){
  mtca4u::LinearStepperMotor::setTargetPosition(newPos);
  mtca4u::LinearStepperMotor::start();
  return true;
}

bool MD22LinearStepperMotor::startMoveRelative(float delta){
  float currentPos = mtca4u::LinearStepperMotor::getCurrentPosition();
  float newPos = currentPos + delta;
  return startMove(newPos);
}

bool MD22LinearStepperMotor::stopMotor(){
  mtca4u::LinearStepperMotor::stop();
  return true;
}

bool MD22LinearStepperMotor::emergencyStopMotor(){
  mtca4u::LinearStepperMotor::emergencyStop();
  return true;
}

bool MD22LinearStepperMotor::isCalibrated(){
  return _isCalibrated;
}

bool MD22LinearStepperMotor::motorIsBusy(){
  if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    return isMoving();
  }else{
    return true;
  }
}

int MD22LinearStepperMotor::getStatus(){
  mtca4u::LinearStepperMotorStatusAndError statusAndError = getStatusAndError();
  return statusAndError.status.getId();
}

int MD22LinearStepperMotor::getError(){
  mtca4u::LinearStepperMotorStatusAndError statusAndError = getStatusAndError();
  return statusAndError.error.getId();
}

float MD22LinearStepperMotor::getCurrentPosition(){
  return mtca4u::LinearStepperMotor::getCurrentPosition();
}

int MD22LinearStepperMotor::getCurrentPositionInSteps(){
  return recalculateUnitsToSteps(mtca4u::LinearStepperMotor::getCurrentPosition());
}

bool MD22LinearStepperMotor::startCalibrate(){
  if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    _future = std::async(std::launch::async, &MD22LinearStepperMotor::calibrateThread, this);
    return true;
  }else{
    return false;
  }
}

void MD22LinearStepperMotor::calibrateThread(){
  mtca4u::LinearStepperMotor::calibrateMotor();
  mtca4u::LinearStepperMotor::moveToPosition(mtca4u::LinearStepperMotor::getNegativeEndSwitchPosition() - 1000);
  mtca4u::LinearStepperMotor::setCurrentPositionAs(0);
  _isCalibrated = true;
}

float MD22LinearStepperMotor::getPositiveEndSwitchPosition(){
  return mtca4u::LinearStepperMotor::getPositiveEndSwitchPosition();
}

float MD22LinearStepperMotor::getNegativeEndSwitchPosition(){
  return mtca4u::LinearStepperMotor::getNegativeEndSwitchPosition();
}

void MD22LinearStepperMotor::setUnitsConverter(boost::shared_ptr<mtca4u::StepperMotorUnitsConverter>& unitsConverter){
  StepperMotor::setUnitsConverter(unitsConverter);
  setStepperMotorUnitsConverter(unitsConverter);
}

void MD22LinearStepperMotor::setSoftLimitsMotorPosition(float maxPos, float minPos){
  StepperMotor::setSoftLimitsMotorPosition(maxPos, minPos);
  mtca4u::LinearStepperMotor::setMaxPositionLimit(maxPos);
  mtca4u::LinearStepperMotor::setMinPositionLimit(minPos);
}

void MD22LinearStepperMotor::getSoftLimitsMotorPosition(float &maxPos, float &minPos){
  maxPos = mtca4u::LinearStepperMotor::getMaxPositionLimit();
  minPos = mtca4u::LinearStepperMotor::getMinPositionLimit();
}

void MD22LinearStepperMotor::enableSoftLimits(bool enable){
  mtca4u::LinearStepperMotor::setSoftwareLimitsEnabled(enable);
}

bool MD22LinearStepperMotor::isSoftLimitsEnabled(){
  return mtca4u::LinearStepperMotor::getSoftwareLimitsEnabled();
}

void MD22LinearStepperMotor::setEnabled(bool enable){
  mtca4u::LinearStepperMotor::setEnabled(enable);
}

bool MD22LinearStepperMotor::getEnabled(){
  return mtca4u::LinearStepperMotor::getEnabled();
}

void MD22LinearStepperMotor::setUserSpeedLimit(double newSpeed){
  mtca4u::LinearStepperMotor::setUserSpeedLimit(newSpeed);
}

double MD22LinearStepperMotor::getUserSpeedLimit(){
  return mtca4u::LinearStepperMotor::getUserSpeedLimit();
}

DummyLinearStepperMotor::DummyLinearStepperMotor(int bamMotorID,
                                                 std::string motorDriverCardDeviceName,
                                                 std::string moduleName,
                                                 unsigned int motorDriverId,
                                                 std::string motorDriverCardConfigFileName) :
                				        MD22LinearStepperMotor(bamMotorID,
                				                               motorDriverCardDeviceName,
                				                               moduleName,
                				                               motorDriverId,
                				                               motorDriverCardConfigFileName){
  _myDummyController = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorControler);
  if (!_myDummyController) throw StepperMotorException(EXCEPTION_MESSAGE + std::string(", pointer for Dummy null"));
}

DummyLinearStepperMotor::~DummyLinearStepperMotor(){}

void DummyLinearStepperMotor::moveToPosition (float newPosition)
{
  boost::thread workedThread(&DummyLinearStepperMotor::threadMove, this);
  mtca4u::LinearStepperMotor::moveToPosition (newPosition);
  workedThread.join();
}

void DummyLinearStepperMotor::threadMove()
{
  //sleep between threadMoveToPostion start and first command to give time for run moveToPosition
  usleep(10000);
  // simulate the movement
  _myDummyController->moveTowardsTarget(1, false);
}

BeckhoffLinearStepperMotor::BeckhoffLinearStepperMotor(int bamMotorID, std::string motorServerAddress) :
        StepperMotor(bamMotorID),
        _targetPosition(0),
        _connectionProblem(false),
        _notResponding(false),
        _softPositiveLimitReached(false),
        _softNegativeLimitReached(false),
        _startMoveTime(0),
        _presentTime(0),
        _startMoveSecondTry(false),
        _isCalibrated(false),
        _softLimitsEnabled(false),
        _calibrationIsRunning(false),
        _positiveEndSwitchPositionInSteps(0),
        _negativeEndSwitchPositionInSteps(0),
        _future(),
        _mutex(),
        _motorClient(motorServerAddress),
        _enabled(true){
  _targetPosition = getCurrentPositionInSteps();
}

BeckhoffLinearStepperMotor::~BeckhoffLinearStepperMotor(){}

bool BeckhoffLinearStepperMotor::startMove(float newPosFemntoSec){
  std::lock_guard<std::mutex> guard(_mutex);
  if ((!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)) && _enabled){
    _future = std::async(std::launch::async, &BeckhoffLinearStepperMotor::moveThread, this, newPosFemntoSec);
    return true;
  }
  else{
    return false;
  }
}

bool BeckhoffLinearStepperMotor::startMoveRelative(float delta){
  std::lock_guard<std::mutex> guard(_mutex);
  if ((!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)) && _enabled){
    _future = std::async(std::launch::async, &BeckhoffLinearStepperMotor::moveRelativeThread, this, delta);
    return true;
  }else{
    return false;
  }
}

bool BeckhoffLinearStepperMotor::startCalibrate(){
  std::lock_guard<std::mutex> guard(_mutex);
  if ((!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)) && _enabled){
    _calibrationIsRunning = true;
    _future = std::async(std::launch::async, &BeckhoffLinearStepperMotor::calibrateThread, this);
    return true;
  }else{
    return false;
  }
}


bool BeckhoffLinearStepperMotor::motorIsBusy(){
  std::lock_guard<std::mutex> guard(_mutex);
  if (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) != std::future_status::ready){
    return true;
  }else{
    return false;
  }
}

void BeckhoffLinearStepperMotor::calibrateThread(){
  enableSoftLimits(false);

  int index = 0;

  while (getStatus() != MOTOR_HARD_NEG_LIMIT_REACHED){
    move(-10000 * index);
    while(actionRunning()){}
    if (interruptCalibration()){
      return;
    }
    index++;
  }

  index = 1;

  while (getStatus() != MOTOR_HARD_POS_LIMIT_REACHED){
    move(500000 * index);
    while(actionRunning()){}
    if (interruptCalibration()){
      return;
    }
    index++;
  }

  float endSwitchPositiveTemp = getCurrentPositionInSteps();

  move(0);
  while(actionRunning()){}
  if (interruptCalibration()){
    return;
  }

  _isCalibrated = true;
  _positiveEndSwitchPositionInSteps = endSwitchPositiveTemp;
  _negativeEndSwitchPositionInSteps = 0;
}

bool BeckhoffLinearStepperMotor::interruptCalibration(){
  if (!_calibrationIsRunning){
    return true;
  }else if (getStatus() != MOTOR_OK &&
      getStatus() != MOTOR_HARD_NEG_LIMIT_REACHED &&
      getStatus() != MOTOR_HARD_POS_LIMIT_REACHED){
    _calibrationIsRunning = false;
    return true;
  }else{
    return false;
  }
}

void BeckhoffLinearStepperMotor::moveThread(float newPosFemntoSec){
  int newPos = _unitsConverter->unitsToSteps(newPosFemntoSec);
  checkLimits(newPos);
  move(newPos);
  while(actionRunning()){}
  return;
}

void BeckhoffLinearStepperMotor::moveRelativeThread(float delta){
  int newPosition = _unitsConverter->unitsToSteps(getCurrentPosition() + delta);
  checkLimits(newPosition);
  move(newPosition);
  while(actionRunning()){}
  return;
}

bool BeckhoffLinearStepperMotor::actionRunning(){
  int status;
  int cmd;

  try{
      status = _motorClient.get<int>("MSTATUS");
      cmd    = _motorClient.get<int>("CMD");
      _connectionProblem = false;
  }catch(StepperMotorException &ex){
      _connectionProblem = true;
      return false;
  }

  if (cmd == 2){
      std::this_thread::sleep_for (std::chrono::seconds(1));
      _targetPosition = getCurrentPositionInSteps();
  }

  if ((status == 0x14) || (status == 0x24)){
      std::this_thread::sleep_for (std::chrono::seconds(1));
      try{
	  status = _motorClient.get<int>("MSTATUS");
	  _connectionProblem = false;
      }catch(StepperMotorException &ex){
	  _connectionProblem = true;
	  return false;
      }
  }

  if (((status & 0x04) && inPosition()) || //TARGET POSITION REACHED
      (status == 0x14) || //POSITIVE END SWITCH ON
      (status == 0x24) || //NEGATIVE END SWITCH ON
      (status & 0x08)){ //ERROR FOUND
      return false;
  }

  return !isTimeOut();
}

bool BeckhoffLinearStepperMotor::isTimeOut(){
  time(&_presentTime);

  if (fabs(_presentTime - _startMoveTime) > 30){

    if (_startMoveSecondTry){
      _notResponding = true;
      return true;
    }else{
      try {
        _motorClient.set<int>("CMD", 1);
        time(&_startMoveTime);
        _startMoveSecondTry = true;
        _connectionProblem = false;
        //std::cout << "second try" << std::endl;
      }catch (StepperMotorException &ex){
        _connectionProblem = true;
        return true;
      }
    }
  }

  return false;
}

void BeckhoffLinearStepperMotor::move(int positionTarget){
  _targetPosition = positionTarget;

  try{
    _motorClient.set<int>("POS.SET", _targetPosition);
    _motorClient.set<int>("CMD", 1);
    _connectionProblem = false;
  }catch(StepperMotorException &ex){
    _connectionProblem = true;
    return;
  }

  time(&_startMoveTime);

  _startMoveSecondTry = false;
  _notResponding      = false;

  return;
}

void BeckhoffLinearStepperMotor::checkLimits(int &newPos){
  _softPositiveLimitReached = false;
  _softNegativeLimitReached = false;

  if (_softLimitsEnabled){
    if (newPos > _maxPos){
      newPos = _maxPos;
      _softPositiveLimitReached = true;
    }
    else if (newPos < _minPos){
      newPos = _minPos;
      _softNegativeLimitReached = true;
    }
  }
}

bool BeckhoffLinearStepperMotor::stopMotor(){
  if (_calibrationIsRunning){
      _calibrationIsRunning = false;
      return true;
  }else{
      try{
	  _motorClient.set<int>("CMD", 2);
	  _connectionProblem = false;
	  return true;
      }catch(StepperMotorException &ex){
	  _connectionProblem = true;
	  return false;
      }
  }
}

bool BeckhoffLinearStepperMotor::emergencyStopMotor(){
  setEnabled(false);
  _targetPosition = getCurrentPositionInSteps();
  return true;
}

bool BeckhoffLinearStepperMotor::isCalibrated(){
  return _isCalibrated;
}

float BeckhoffLinearStepperMotor::getPositiveEndSwitchPosition(){
  return _positiveEndSwitchPositionInSteps;
}

float BeckhoffLinearStepperMotor::getNegativeEndSwitchPosition(){
  return _negativeEndSwitchPositionInSteps;
}

void BeckhoffLinearStepperMotor::enableSoftLimits(bool enable){
  std::lock_guard<std::mutex> guard(_mutex);
  if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    _softLimitsEnabled = enable;
  }
}

bool BeckhoffLinearStepperMotor::isSoftLimitsEnabled(){
  return _softLimitsEnabled;
}

void BeckhoffLinearStepperMotor::setEnabled(bool enable){
  std::lock_guard<std::mutex> guard(_mutex);
  if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    enable = _enabled;
  }
}

bool BeckhoffLinearStepperMotor::getEnabled(){
  return _enabled;
}

int BeckhoffLinearStepperMotor::getError(){
  uint32_t errorBitField = 0;
  int status;
  try{
    status = _motorClient.get<int>("MSTATUS");
    _connectionProblem = false;
    if (_notResponding){
      errorBitField = errorBitField | MOTOR_NOT_RESPONDING;
    }
    else if (!(status & 0x8)){
      errorBitField = MOTOR_NO_ERROR;
    }else {
      errorBitField = errorBitField | MOTOR_NOT_RESPONDING;
    }
  }catch(StepperMotorException &ex){
    errorBitField = errorBitField | MOTOR_COMMUNICATION_LOST;
    if (_notResponding){
      errorBitField = errorBitField | MOTOR_NOT_RESPONDING;
    }
  }

  return errorBitField;
}

int BeckhoffLinearStepperMotor::getStatus(){
  if (_connectionProblem || _notResponding){
    return MOTOR_ERROR;
  }

  int status;
  try{
    status = _motorClient.get<int>("MSTATUS");
    _connectionProblem = false;
  }catch(StepperMotorException &ex){
    _connectionProblem = true;
    return MOTOR_ERROR;
  }

  if (_softLimitsEnabled){
    if (_softPositiveLimitReached){
      return MOTOR_SOFT_POS_LIMIT_REACHED;
    }
    else if (_softNegativeLimitReached){
      return MOTOR_SOFT_NEG_LIMIT_REACHED;
    }
  }

  if ((status & 0x10) && (status & 0x4)){
    return MOTOR_HARD_POS_LIMIT_REACHED;
  }
  else if ((status & 0x20) && (status & 0x4)){
    return MOTOR_HARD_NEG_LIMIT_REACHED;
  }
  else if ((status & 0x4) && inPosition()){
    return MOTOR_OK;
  }
  else if (status & 0x3){
    return MOTOR_IN_MOVE;
  }
  else if ((status & 0x4) && !inPosition()){
    return MOTOR_NOT_IN_POSITION;
  }
  else if (status & 0x8){
    return MOTOR_ERROR;
  }
  return MOTOR_ERROR;
}

float BeckhoffLinearStepperMotor::getCurrentPosition(){
  return _unitsConverter->stepsToUnits(getCurrentPositionInSteps());
}

int BeckhoffLinearStepperMotor::getCurrentPositionInSteps(){
  int position;
  try{
    position = _motorClient.get<int>("POS");
    _connectionProblem = false;
    return position;
  }catch(StepperMotorException &ex){
    _connectionProblem = true;
    return std::numeric_limits<int>::min();
  }
}


MotorUnitsConverter::~MotorUnitsConverter(){}
