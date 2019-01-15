/*
 * ControlInput.cc
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#include "ControlInput.h"


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
//ControlInput::ControlInput(ctk::EntityOwner *owner,
//                                     const std::string &name,
//                                     const std::string &description)
//  : ctk::VariableGroup{owner, name, description, true} {};

///* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
//LinearMotorControlInput::LinearMotorControlInput(ctk::EntityOwner *owner,
//                                     const std::string &name,
//                                     const std::string &description)
//  : ctk::VariableGroup{owner, name, description, true} {};


ControlInputHandler::ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description, std::shared_ptr<ctk::StepperMotor> motor)
    : ctk::ApplicationModule(owner, name, description),
      _controlInput{this, "controlInput", "Control inputs", true},
      _calibrationCommands{},
      _motor(motor)
{
  // If motor has HW reference switches,
  // calibration is supported
  if(_motor->hasHWReferenceSwitches()){
      _calibrationCommands = CalibrationCommands{this, "calibrationCommands", "Calibration commands", true};
  }
};

void ControlInputHandler::createFunctionMap(std::shared_ptr<ctk::StepperMotor> motor){

  funcMap[_controlInput.enableMotor.getId()]               = [this]{enableCallback();};
  funcMap[_controlInput.disableMotor.getId()]              = [this]{disableCallback();};
  funcMap[_controlInput.positionSetpointInSteps.getId()]   = [this]{setTargetPositionInStepsCallback();};
  funcMap[_controlInput.positionSetpoint.getId()]          = [this]{setTargetPositionCallback();};
  funcMap[_controlInput.startMotor.getId()]                = [this]{startCallback();};
  funcMap[_controlInput.stopMotor.getId()]                 = [this, motor]{ if(_controlInput.stopMotor){motor->stop();} };
  funcMap[_controlInput.emergencyStopMotor.getId()]        = [this, motor]{ if(_controlInput.emergencyStopMotor){ motor->emergencyStop();} };
  funcMap[_controlInput.resetError.getId()]                = [      motor]{ motor->resetError(); };
  funcMap[_controlInput.enableAutostart.getId()]           = [this, motor]{ motor->setAutostart(_controlInput.enableAutostart);};
  funcMap[_controlInput.moveRelativeInSteps.getId()]       = [this, motor]{ motor->moveRelativeInSteps(_controlInput.moveRelativeInSteps); };
  funcMap[_controlInput.moveRelative.getId()]              = [this, motor]{ motor->moveRelative(_controlInput.moveRelative); };
  funcMap[_controlInput.referencePositionInSteps.getId()]  = [this, motor]{ motor->setActualPositionInSteps(_controlInput.referencePositionInSteps); };
  funcMap[_controlInput.referencePosition.getId()]         = [this, motor]{ motor->setActualPosition(_controlInput.referencePosition); };
  funcMap[_controlInput.encoderReferencePosition.getId()]  = [this, motor]{ motor->setActualEncoderPosition(_controlInput.encoderReferencePosition); };
  funcMap[_controlInput.axisTranslationInSteps.getId()]    = [this, motor]{ motor->translateAxisInSteps(_controlInput.axisTranslationInSteps); };
  funcMap[_controlInput.axisTranslation.getId()]           = [this, motor]{ motor->translateAxis(_controlInput.axisTranslation); };
  funcMap[_controlInput.enableSWPositionLimits.getId()]    = [this, motor]{ motor->setSoftwareLimitsEnabled(_controlInput.enableSWPositionLimits); };
  funcMap[_controlInput.maxSWPositionLimit.getId()]        = [this, motor]{ motor->setMaxPositionLimit(_controlInput.maxSWPositionLimit); };
  funcMap[_controlInput.minSWPositionLimit.getId()]        = [this, motor]{ motor->setMinPositionLimit(_controlInput.minSWPositionLimit); };
  funcMap[_controlInput.maxSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMaxPositionLimitInSteps(_controlInput.maxSWPositionLimitInSteps); };
  funcMap[_controlInput.minSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMinPositionLimitInSteps(_controlInput.minSWPositionLimitInSteps); };
  funcMap[_controlInput.currentLimit.getId()]              = [this, motor]{ motor->setUserCurrentLimit(_controlInput.currentLimit); };
  funcMap[_controlInput.speedLimit.getId()]                = [this, motor]{ motor->setUserSpeedLimit(_controlInput.speedLimit); };
  funcMap[_controlInput.enableFullStepping.getId()]        = [this, motor]{ motor->enableFullStepping(_controlInput.enableFullStepping); };
}


void ControlInputHandler::prepare(){
    createFunctionMap(_motor);

    if(_motor->hasHWReferenceSwitches()){
      appendCalibrationToMap();
    }
}

void ControlInputHandler::mainLoop() {

  inputGroup = this->readAnyGroup();

  while(true){

    _controlInput.userMessage = "";

    auto changedVarId = inputGroup.readAny();

  // FIXME This is not thread-safe as the call to isSystemIdle and the funcMap are not atomic
    //FIXME Keep this only as long as we rely on the dummy for tests
    try{
      funcMap.at(changedVarId)();
    }
    catch(mtca4u::MotorDriverException &e){
      _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: " + std::string(e.what());
    }

    _controlInput.dummyMotorStop = _controlInput.stopMotor || _controlInput.emergencyStopMotor;
    _controlInput.dummyMotorTrigger++;

    writeAll();
  }
}

void ControlInputHandler::enableCallback(){
  _motor->setEnabled(true);
}

void ControlInputHandler::disableCallback(){
  _motor->setEnabled(false);
}

void ControlInputHandler::startCallback(){

  if(_controlInput.startMotor){
    if(_motor->isSystemIdle()){
      _motor->start();
    }
    else{
      _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called startMotor while motor is not in IDLE state.";
    }
  }
}

void ControlInputHandler::setTargetPositionCallback(){
  if(_motor->isSystemIdle()){
    _motor->setTargetPosition(_controlInput.positionSetpoint);
  }
  else{
    _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Requested change of target position while motor is not in IDLE state.";
  }
}

void ControlInputHandler::setTargetPositionInStepsCallback(){
  if(_motor->isSystemIdle()){
    _motor->setTargetPositionInSteps(_controlInput.positionSetpointInSteps);
  }
  else{
    _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Requested change of target position while motor is not in IDLE state.";
  }
}


void ControlInputHandler::appendCalibrationToMap(){

  funcMap[_calibrationCommands.calibrateMotor.getId()]     = [this]{calibrateCallback();};
  funcMap[_calibrationCommands.determineTolerance.getId()] = [this]{determineToleranceCallback();};
}

void ControlInputHandler::calibrateCallback(){
  if(_calibrationCommands.calibrateMotor){
    if(_motor->isSystemIdle()){
      _motor->calibrate();
    }
    else{
      ControlInputHandler::_controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called calibrateMotor while motor is not in IDLE state.";
    }
  }
}

void ControlInputHandler::determineToleranceCallback(){
  if(_calibrationCommands.determineTolerance){
    if(_motor->isSystemIdle()){
      _motor->determineTolerance();
    }
    else{
      ControlInputHandler::_controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called determineTolerance while motor is not in IDLE state.";
    }
  }
}
