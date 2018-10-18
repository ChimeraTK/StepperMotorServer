/*
 * ControlInput.cc
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#include "ControlInput.h"


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
BasicControlInput::BasicControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ctk::VariableGroup{owner, name, description, true} {};

/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
LinearMotorControlInput::LinearMotorControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ctk::VariableGroup{owner, name, description, true} {};


void BasicControlInputHandler::createFunctionMap(std::shared_ptr<ctk::StepperMotor> motor){

  funcMap[_controlInput.enableMotor.getId()]               = [this]{enableCallback();};
  funcMap[_controlInput.disableMotor.getId()]               = [this]{disableCallback();};
  funcMap[_controlInput.positionSetpointInSteps.getId()]   = [this]{setTargetPositionInStepsCallback();}; //[this, motor]{ motor->setTargetPositionInSteps(_controlInput.positionSetpointInSteps); };
  funcMap[_controlInput.positionSetpoint.getId()]          = [this]{setTargetPositionCallback();}; //[this, motor]{ motor->setTargetPosition(_controlInput.positionSetpoint); };
  funcMap[_controlInput.startMotor.getId()]                = [this]{startCallback();};//[this, motor]{ if(_controlInput.startMotor){motor->start();} };
  funcMap[_controlInput.stopMotor.getId()]                 = [this, motor]{ if(_controlInput.stopMotor){motor->stop();} };
  funcMap[_controlInput.emergencyStopMotor.getId()]        = [this, motor]{ if(_controlInput.emergencyStopMotor){ motor->emergencyStop();} };
  funcMap[_controlInput.enableAutostart.getId()]           = [this, motor]{ motor->setAutostart(_controlInput.enableAutostart);};
  funcMap[_controlInput.moveRelativeInSteps.getId()]       = [this, motor]{ motor->moveRelativeInSteps(_controlInput.moveRelativeInSteps); };
  funcMap[_controlInput.moveRelative.getId()]              = [this, motor]{ motor->moveRelative(_controlInput.moveRelative); };
  funcMap[_controlInput.referencePositionInSteps.getId()]  = [this, motor]{ motor->setActualPositionInSteps(_controlInput.referencePositionInSteps); };
  funcMap[_controlInput.referencePosition.getId()]         = [this, motor]{ motor->setActualPosition(_controlInput.referencePosition); };
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


void BasicControlInputHandler::mainLoop() {

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

void BasicControlInputHandler::enableCallback(){
  _motor->setEnabled(true);
}

void BasicControlInputHandler::disableCallback(){
  _motor->setEnabled(false);
}

void BasicControlInputHandler::startCallback(){

  if(_controlInput.startMotor){
    if(_motor->isSystemIdle()){
      _motor->start();
    }
    else{
      _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called startMotor while motor is not in IDLE state.";
    }
  }
}

void BasicControlInputHandler::setTargetPositionCallback(){
  if(_motor->isSystemIdle()){
    _motor->setTargetPosition(_controlInput.positionSetpoint);
  }
  else{
    _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Requested change of target position while motor is not in IDLE state.";
  }
}

void BasicControlInputHandler::setTargetPositionInStepsCallback(){
  if(_motor->isSystemIdle()){
    _motor->setTargetPositionInSteps(_controlInput.positionSetpointInSteps);
  }
  else{
    _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Requested change of target position while motor is not in IDLE state.";
  }
}



// Implementation of LinearMotorControlInputHandler functions

void LinearMotorControlInputHandler::createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> motor){

  funcMap[_controlInput.calibrateMotor.getId()]            = [this]{calibrateCallback();};

  funcMap[BasicControlInputHandler::_controlInput.axisTranslationInSteps.getId()]
    = [this, motor]{ motor->translateAxisInSteps(BasicControlInputHandler::_controlInput.axisTranslationInSteps); };
  funcMap[BasicControlInputHandler::_controlInput.axisTranslation.getId()]
    = [this, motor]{ motor->translateAxis(BasicControlInputHandler::_controlInput.axisTranslation); };

  funcMap[_controlInput.determineTolerance.getId()] = [this]{determineToleranceCallback();};
}

void LinearMotorControlInputHandler::calibrateCallback(){
  if(_controlInput.calibrateMotor){
    if(_motor->isSystemIdle()){
      _motor->calibrate();
    }
    else{
      BasicControlInputHandler::_controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called calibrateMotor while motor is not in IDLE state.";
    }
  }
}

void LinearMotorControlInputHandler::determineToleranceCallback(){
  if(_controlInput.determineTolerance){
    if(_motor->isSystemIdle()){
      _motor->determineTolerance();
    }
    else{
      BasicControlInputHandler::_controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Called determineTolerance while motor is not in IDLE state.";
    }
  }
}
