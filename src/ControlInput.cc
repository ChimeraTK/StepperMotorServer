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

  funcMap[_controlInput.enableMotor.getId()]               = [this, motor]{ motor->setEnabled(_controlInput.enableMotor); };
  funcMap[_controlInput.positionSetpointInSteps.getId()]   = [this, motor]{ motor->setTargetPositionInSteps(_controlInput.positionSetpointInSteps); };
  funcMap[_controlInput.positionSetpoint.getId()]          = [this, motor]{ motor->setTargetPosition(_controlInput.positionSetpoint); };
  funcMap[_controlInput.startMotor.getId()]                = [this, motor]{ if(_controlInput.startMotor){motor->start();} };
  funcMap[_controlInput.stopMotor.getId()]                 = [this, motor]{ if(_controlInput.stopMotor){motor->stop();} };
  funcMap[_controlInput.emergencyStopMotor.getId()]        = [this, motor]{ if(_controlInput.emergencyStopMotor){ motor->emergencyStop();} };
  funcMap[_controlInput.enableAutostart.getId()]           = [this, motor]{ motor->setAutostart(_controlInput.enableAutostart);};
  funcMap[_controlInput.moveRelativeInSteps.getId()]       = [this, motor]{ motor->moveRelativeInSteps(_controlInput.moveRelativeInSteps); };
  funcMap[_controlInput.moveRelative.getId()]              = [this, motor]{ motor->moveRelative(_controlInput.moveRelative); };
  funcMap[_controlInput.enableSWPositionLimits.getId()]    = [this, motor]{ motor->setSoftwareLimitsEnabled(_controlInput.enableSWPositionLimits); };
  funcMap[_controlInput.maxSWPositionLimit.getId()]        = [this, motor]{ motor->setMaxPositionLimit(_controlInput.maxSWPositionLimit); };
  funcMap[_controlInput.minSWPositionLimit.getId()]        = [this, motor]{ motor->setMinPositionLimit(_controlInput.minSWPositionLimit); };
  funcMap[_controlInput.maxSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMaxPositionLimitInSteps(_controlInput.maxSWPositionLimitInSteps); };
  funcMap[_controlInput.minSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMinPositionLimitInSteps(_controlInput.minSWPositionLimitInSteps); };
  funcMap[_controlInput.currentLimit.getId()]              = [this, motor]{ motor->setUserCurrentLimit(_controlInput.currentLimit); };
  funcMap[_controlInput.speedLimit.getId()]                = [this, motor]{ motor->setUserSpeedLimit(_controlInput.speedLimit); };
  funcMap[_controlInput.enableFullStepping.getId()]        = [this, motor]{ motor->enableFullStepping(); };
}


void BasicControlInputHandler::mainLoop() {

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  while(true){

    auto changedVarId = inputGroup.readAny();

    // FIXME This is not thread-safe as the call to isSystemIdle and the funcMap are not atomic
    if(_motor->isSystemIdle()
        || changedVarId == _controlInput.stopMotor.getId() || changedVarId == _controlInput.emergencyStopMotor.getId()){
      //FIXME Keep this only as long as we rely on the dummy for tests
      try{
        funcMap.at(changedVarId)();
      }
      catch(mtca4u::MotorDriverException &e){
        _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: " + std::string(e.what());
      }
      _controlInput.userMessage = "";
    }
    else{
      _controlInput.userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
    }

    _controlInput.dummyMotorStop = _controlInput.stopMotor || _controlInput.emergencyStopMotor;
    _controlInput.dummyMotorTrigger++;

    writeAll();
  }
}


void LinearMotorControlInputHandler::createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor){

  funcMap[_controlInput.calibrateMotor.getId()] = [this, _motor]{_motor->calibrate();};
}
