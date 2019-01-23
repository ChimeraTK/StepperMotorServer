/*
 * ControlInput.cc
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#include "ControlInput.h"



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

  funcMap[control.enable.getId()]                    = [this]{enableCallback();};
  funcMap[control.disable.getId()]                   = [this]{disableCallback();};
  funcMap[_controlInput.positionSetpointInSteps.getId()]   = [this]{setTargetPositionInStepsCallback();};
  funcMap[_controlInput.positionSetpoint.getId()]          = [this]{setTargetPositionCallback();};
  funcMap[_controlInput.startMotor.getId()]                = [this]{startCallback();};
  funcMap[control.stop.getId()]                            = [this, motor]{ if(control.stop){motor->stop();} };
  funcMap[control.emergencyStop.getId()]        = [this, motor]{ if(control.emergencyStop){ motor->emergencyStop();} };
  funcMap[control.resetError.getId()]                = [      motor]{ motor->resetError(); };
  funcMap[control.enableAutostart.getId()]           = [this, motor]{ motor->setAutostart(control.enableAutostart);};
  funcMap[_controlInput.moveRelativeInSteps.getId()]       = [this, motor]{ motor->moveRelativeInSteps(_controlInput.moveRelativeInSteps); };
  funcMap[_controlInput.moveRelative.getId()]              = [this, motor]{ motor->moveRelative(_controlInput.moveRelative); };
  funcMap[_controlInput.referencePositionInSteps.getId()]  = [this, motor]{ motor->setActualPositionInSteps(_controlInput.referencePositionInSteps); };
  funcMap[_controlInput.referencePosition.getId()]         = [this, motor]{ motor->setActualPosition(_controlInput.referencePosition); };
  funcMap[_controlInput.encoderReferencePosition.getId()]  = [this, motor]{ motor->setActualEncoderPosition(_controlInput.encoderReferencePosition); };
  funcMap[_controlInput.axisTranslationInSteps.getId()]    = [this, motor]{ motor->translateAxisInSteps(_controlInput.axisTranslationInSteps); };
  funcMap[_controlInput.axisTranslation.getId()]           = [this, motor]{ motor->translateAxis(_controlInput.axisTranslation); };
  funcMap[swLimits.enable.getId()]               = [this, motor]{ motor->setSoftwareLimitsEnabled(swLimits.enable); };
  funcMap[swLimits.maxPosition.getId()]                    = [this, motor]{ motor->setMaxPositionLimit(swLimits.maxPosition); };
  funcMap[swLimits.minPosition.getId()]        = [this, motor]{ motor->setMinPositionLimit(swLimits.minPosition); };
  funcMap[swLimits.maxPositionInSteps.getId()] = [this, motor]{ motor->setMaxPositionLimitInSteps(swLimits.maxPositionInSteps); };
  funcMap[swLimits.minPositionInSteps.getId()] = [this, motor]{ motor->setMinPositionLimitInSteps(swLimits.minPositionInSteps); };
  funcMap[_controlInput.currentLimit.getId()]              = [this, motor]{ motor->setUserCurrentLimit(_controlInput.currentLimit); };
  funcMap[_controlInput.speedLimit.getId()]                = [this, motor]{ motor->setUserSpeedLimit(_controlInput.speedLimit); };
  funcMap[control.enableFullStepping.getId()]        = [this, motor]{ motor->enableFullStepping(control.enableFullStepping); };
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
    _controlInput.userWarning = 0;

    auto changedVarId = inputGroup.readAny();

    //FIXME Keep this only as long as we rely on the dummy for tests
    try{
      funcMap.at(changedVarId)();
    }
    catch(mtca4u::MotorDriverException &e){
      _controlInput.userMessage = "Exception: " + std::string(e.what());
    }

    if(std::string("").compare(_controlInput.userMessage)){
      _controlInput.userWarning = 1;
    }

    _controlInput.dummyMotorStop = control.stop || control.emergencyStop;
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
  _motor->setTargetPosition(_controlInput.positionSetpoint);
}

void ControlInputHandler::setTargetPositionInStepsCallback(){
  _motor->setTargetPositionInSteps(_controlInput.positionSetpointInSteps);
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
