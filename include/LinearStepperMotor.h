#include <mtca4u/MotorDriverCard/LinearStepperMotor.h>
#include <mtca4u/MotorDriverCard/MotorControlerDummy.h>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <atomic>
#include <future>
#include <thread>
#include <time.h>
#include <eq_client.h> // to be able to read from other DOOCS server
#include "DOOCSClient.h"

#ifndef LINEARSTEPPERMOTOR_H
#define LINEARSTEPPERMOTOR_H

class BeckhoffMotorTest;

class MotorUnitsConverter : public mtca4u::StepperMotorUnitsConverter{
 public:
 MotorUnitsConverter(float convFactor = 1, int reflection = 1, double traslationOffset = 0):
  _convFactor(convFactor),
    _reflection(reflection),
    _traslationOffset(traslationOffset){};
  virtual float stepsToUnits(int steps) {return steps * _convFactor * _reflection + _traslationOffset;};
  virtual int unitsToSteps(float unity){
    int steps = (unity-_traslationOffset)/(_convFactor * _reflection); 
    return steps;
  };
  virtual ~MotorUnitsConverter();
 private:
  float _convFactor;
  int   _reflection;
  double _traslationOffset;
};

class StepperMotor{
public:
  StepperMotor(int bamMotorID);

  static const int MOTOR_OK;
  static const int MOTOR_DISABLED;
  static const int MOTOR_IN_MOVE;
  static const int MOTOR_NOT_IN_POSITION;
  static const int MOTOR_SOFT_POS_LIMIT_REACHED;
  static const int MOTOR_SOFT_NEG_LIMIT_REACHED;
  static const int MOTOR_ERROR;
  static const int MOTOR_HARD_POS_LIMIT_REACHED;
  static const int MOTOR_HARD_NEG_LIMIT_REACHED;

  static const int MOTOR_NO_ERROR;
  static const int MOTOR_COMMUNICATION_LOST;
  static const int MOTOR_CONFIG_ERROR;
  static const int MOTOR_NOT_RESPONDING;
  static const int MOTOR_HARDWARE_NOT_CONNECTED;
  static const int MOTOR_BOTH_END_SWITCH_ON;

  virtual ~StepperMotor();
  virtual bool   startMove(float newPos) = 0;
  virtual bool   startMoveRelative(float delta) = 0;
  virtual bool   startCalibrate() = 0;
  virtual bool   stopMotor() = 0;
  virtual bool   emergencyStopMotor() = 0;
  virtual bool   motorIsBusy() = 0;
  virtual bool   isCalibrated() = 0;
  virtual float  getPositiveEndSwitchPosition() = 0;
  virtual float  getNegativeEndSwitchPosition() = 0;
  virtual float  getCurrentPosition() = 0;
  virtual int    getCurrentPositionInSteps() = 0;
  virtual int    getError() = 0;
  virtual int    getStatus() = 0;
  virtual void   setSoftLimitsMotorPosition(float maxPos, float minPos);
  virtual void   getSoftLimitsMotorPosition(float &maxPos, float &minPos);
  virtual void   enableSoftLimits(bool enable) = 0;
  virtual bool   isSoftLimitsEnabled() = 0;
  virtual void   setEnabled(bool enable) = 0;
  virtual bool 	 getEnabled() = 0;
  virtual void 	 setUserSpeedLimit(double newSpeed) = 0;
  virtual double getUserSpeedLimit() = 0;
  virtual void   setUnitsConverter(boost::shared_ptr<mtca4u::StepperMotorUnitsConverter> &unitsConverter);
  virtual void   initPos(float newPos){}
  int            getMotorId(){return _motorID;}

  // FIXME BAM_RELATED
  static const double mmPerFemtoseconds;

  /** A creator function to abstract if a dummy or a real stepper motor is to be created.
   *  It has the same signature as the constructor.
   */
  static boost::shared_ptr<StepperMotor> create(int bamMotorID,
                                                   std::string motorDriverCardDeviceName = "",
						   std::string moduleName = "",
                                                   unsigned int motorDriverId = 0,
                                                   std::string motorDriverCardConfigFileName = "",
                                                   std::string addressMotorServer = "");

protected:
  int _motorID;
  int _maxPos, _minPos;
  boost::shared_ptr<mtca4u::StepperMotorUnitsConverter> _unitsConverter;
};

class StepperMotorImp : public StepperMotor{
public:
  StepperMotorImp(int bamMotorID):
    StepperMotor(bamMotorID),
    _currentPos(0),
    _enabled(true),
    _speedLimit(1e8),
    _isCalibrated(false),
    _isSoftLimitsenabled(false)
{}
  virtual bool startMove(float newPos) {
    if (_isSoftLimitsenabled){
	if (newPos > _maxPos){
	    newPos = _maxPos;
	}
	if (newPos < _minPos){
	    newPos = _minPos;
	}
    }
    _currentPos = newPos;
    return true;
  }
  virtual bool motorIsBusy() {return false;};
  virtual int getError() {return 1;};
  virtual int getStatus(){return 1;};
  virtual float getCurrentPosition() {return _currentPos;};
  virtual float getTargetPosition() {return _currentPos;};
  //////
  virtual bool startMoveRelative(float delta){
    _currentPos +=  delta;
    if (_isSoftLimitsenabled){
	if (_currentPos > _maxPos){
	    _currentPos = _maxPos;
	}
	if (_currentPos < _minPos){
	    _currentPos = _minPos;
	}
    }
    return true;
  };
  virtual bool startCalibrate() {
    _isCalibrated = true;
    return true;
  }
  virtual bool stopMotor(){return true;}
  virtual bool emergencyStopMotor(){
    _enabled = false;
    return true;
  }
  virtual bool isCalibrated(){return _isCalibrated;}
  virtual float getPositiveEndSwitchPosition(){
    return 1e8;
  };
  virtual float getNegativeEndSwitchPosition(){
    return -1e8;
  }
  virtual int   getCurrentPositionInSteps(){
    return _unitsConverter->unitsToSteps(_currentPos);
  }
  virtual void setEnabled(bool enable){_enabled = enable;}
  virtual bool getEnabled(){return _enabled;}
  virtual void   enableSoftLimits(bool enable){_isSoftLimitsenabled = enable;}
  virtual bool   isSoftLimitsEnabled(){return _isSoftLimitsenabled;}
  virtual void setUserSpeedLimit(double newSpeed){_speedLimit = newSpeed;}
  virtual double getUserSpeedLimit(){return _speedLimit;}
  ////
  virtual ~StepperMotorImp(){}
private:
  float _currentPos;
  bool _enabled;
  double _speedLimit;
  bool _isCalibrated;
  bool _isSoftLimitsenabled;
};

class MD22LinearStepperMotor : public mtca4u::LinearStepperMotor, public StepperMotor{
public:
  MD22LinearStepperMotor(int bamMotorID,
                         std::string motorDriverCardDeviceName,
			 std::string moduleName,
                         unsigned int motorDriverId,
                         std::string motorDriverCardConfigFileName);
  virtual bool   startMove(float newPos);
  virtual bool   startMoveRelative(float delta);
  virtual bool   startCalibrate();
  virtual bool   stopMotor();
  virtual bool   emergencyStopMotor();
  virtual bool   motorIsBusy();
  virtual bool   isCalibrated();
  virtual float  getPositiveEndSwitchPosition();
  virtual float  getNegativeEndSwitchPosition();
  virtual float  getCurrentPosition();
  virtual int    getCurrentPositionInSteps();
  virtual void   initPos(float newPos){mtca4u::LinearStepperMotor::setCurrentPositionAs(newPos);}
  virtual int    getError();
  virtual int    getStatus();
  virtual void   setSoftLimitsMotorPosition(float maxPos, float minPos);
  virtual void   getSoftLimitsMotorPosition(float &maxPos, float &minPos);
  virtual void   enableSoftLimits(bool enable);
  virtual bool   isSoftLimitsEnabled();
  virtual void   setEnabled(bool enable);
  virtual bool 	 getEnabled();
  virtual void 	 setUserSpeedLimit(double newSpeed);
  virtual double getUserSpeedLimit();
  virtual void   setUnitsConverter(boost::shared_ptr<mtca4u::StepperMotorUnitsConverter>& unitsConverter);
  virtual ~MD22LinearStepperMotor();



protected:
  void calibrateThread();
  bool _isCalibrated;
  std::future<void> _future; 
};

class DummyLinearStepperMotor : public MD22LinearStepperMotor{
public:
  DummyLinearStepperMotor(int bamMotorID,
                          std::string motorDriverCardDeviceName,
			  std::string moduleName,
                          unsigned int motorDriverId,
                          std::string motorDriverCardConfigFileName);
  void moveToPosition (float newPosition);
  virtual ~DummyLinearStepperMotor();

protected:
  boost::shared_ptr<mtca4u::MotorControlerDummy> _myDummyController;
  void threadMove();
};

class BeckhoffLinearStepperMotor : public StepperMotor{
public:
  BeckhoffLinearStepperMotor(int bamMotorID, std::string motorServerAddress);
  virtual bool   startMove(float newPosFemtoSec);
  virtual bool   startMoveRelative(float delta);
  virtual bool   startCalibrate();
  virtual bool   stopMotor();
  virtual bool   emergencyStopMotor();
  virtual bool   motorIsBusy();
  virtual bool   isCalibrated();
  virtual float  getPositiveEndSwitchPosition();
  virtual float  getNegativeEndSwitchPosition();
  virtual float  getCurrentPosition();
  virtual int    getCurrentPositionInSteps();
  virtual int    getError();
  virtual int    getStatus();
  virtual void   enableSoftLimits(bool enable);
  virtual bool   isSoftLimitsEnabled();
  virtual void   setEnabled(bool enable);
  virtual bool 	 getEnabled();
  virtual void 	 setUserSpeedLimit(double newSpeed){}
  virtual double getUserSpeedLimit(){return 1e8;}
  virtual ~BeckhoffLinearStepperMotor();

  friend class BeckhoffMotorTest;

private:
  int _targetPosition;
  bool _connectionProblem;
  bool _notResponding;
  bool _softPositiveLimitReached;
  bool _softNegativeLimitReached;
  time_t _startMoveTime;
  time_t _presentTime;
  bool _startMoveSecondTry;
  bool _isCalibrated;
  bool _softLimitsEnabled;
  std::atomic<bool> _calibrationIsRunning;
  int _positiveEndSwitchPositionInSteps;
  int _negativeEndSwitchPositionInSteps;
  std::future<void> _future;
  std::mutex _mutex;
  DOOCSClient _motorClient;
  bool _enabled;
  void calibrateThread();
  void moveThread(float newPosFemntoSec);
  void moveRelativeThread(float delta);
  void move(int positionTarget);
  void checkLimits(int &newPos);
  bool actionRunning();
  bool interruptCalibration();
  bool isTimeOut();
  bool inPosition(){
    if(abs(getCurrentPositionInSteps() - _targetPosition) < 7){
      return true;
    }else{
      return false;
    }
  }
};

#endif
