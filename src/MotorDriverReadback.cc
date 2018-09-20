/*
 * MotorDriverReadback.cc
 *
 *  Created on: Sep 17, 2018
 *      Author: ckampm
 */

#include "MotorDriverReadback.h"

//void HWReadbackValues::readback(){
//
//  // Read from HW
//  isCalibrated = _motor->isCalibrated() ? 1 : 0;
//  ctk::StepperMotorError error = _motor->getError();
//  motorErrorId = static_cast<int32_t>(error);
//  actualPositionInSteps = _motor->getCurrentPositionInSteps();
//  decoderPosition = _motor->getDecoderPosition();
//
//  // Update values that have a static relation to HW readback
//  actualPosition = _motor->recalculateStepsInUnits(actualPositionInSteps);
//
//  // FIXME Debug RBVs
//  enabledRBV = _motor->getEnabled();
//  targetPositionInStepsRBV = _motor->getTargetPositionInSteps();
//
//}
//
//void HWReadbackValuesExt::readback(){
//
//  HWReadbackValues::readback();
//  isNegativeReferenceActive = _motor->isNegativeReferenceActive();
//  isPositiveReferenceActive = _motor->isPositiveReferenceActive();
//}


ReadbackHandler::ReadbackHandler(/*std::shared_ptr<ctk::StepperMotor> motor,*/
                                 ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true) /*,_motor(motor) */{


//    _readbackValues = std::move(std::unique_ptr<HWReadbackValuesExt>(new HWReadbackValuesExt(_motor, this, "hwReadbackValues", "Basic values read from the HW")));
//
//    _readbackValues = std::move(std::unique_ptr<HWReadbackValues>(new HWReadbackValues(_motor, this, "hwReadbackValues", "Basic values read from the HW")));
}

void ReadbackHandler::mainLoop(){

  while(true){
    //Wait for cyclic trigger
    trigger.read();

    // FIXME This is only to evaluate the timer
    if(!execTimer.isInitialized()){
      execTimer.initializeMeasurement();
    }
    else{
      execTimer.measureIterativeMean();
    }
    auto ct = std::chrono::duration_cast<std::chrono::microseconds>(execTimer.getMeasurementResult());
    actualCycleTime = static_cast<float>(ct.count())/1000.f;

    receiveTimer.initializeMeasurement();

    //_readbackValues->readback();
    readbackFunction();

    receiveTimer.measureOnce();
    auto rt = std::chrono::duration_cast<std::chrono::microseconds>(receiveTimer.getMeasurementResult());
    actualReceiveTime = static_cast<float>(rt.count())/1000.f;

    writeAll();
  }
}


BasicHWReadbackHandler::BasicHWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor,
                                               ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ReadbackHandler(owner, name, description), _motor(motor), hwReadbackValues{this, "hwReadbackValues", "Variables read from the HW of the MotorDriver"} {}

void BasicHWReadbackHandler::prepare(){
  readbackFunction = std::bind(&BasicHWReadbackHandler::readback, this);
}

void BasicHWReadbackHandler::readback(){
  std::cout << "   ** BasicHWReadbackHandler::readback() called in " <<   this->_name << std::endl;
    // Read from HW
    hwReadbackValues.isCalibrated = _motor->isCalibrated() ? 1 : 0;
    ctk::StepperMotorError error = _motor->getError();
    hwReadbackValues.motorErrorId = static_cast<int32_t>(error);
    hwReadbackValues.actualPositionInSteps = _motor->getCurrentPositionInSteps();
    hwReadbackValues.decoderPosition = _motor->getDecoderPosition();

    // Update values that have a static relation to HW readback
    hwReadbackValues.actualPosition = _motor->recalculateStepsInUnits(hwReadbackValues.actualPositionInSteps);

    // FIXME Debug RBVs
    hwReadbackValues.enabledRBV = _motor->getEnabled();
    hwReadbackValues.targetPositionInStepsRBV = _motor->getTargetPositionInSteps();

  }

//  void HWReadbackValuesExt::readback(){
//
//    HWReadbackValues::readback();
//    isNegativeReferenceActive = _motor->isNegativeReferenceActive();
//    isPositiveReferenceActive = _motor->isPositiveReferenceActive();
//}


ExtHWReadbackHandler::ExtHWReadbackHandler(std::shared_ptr<ctk::StepperMotorWithReference> motor,
                                            ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : BasicHWReadbackHandler(motor, owner, name, description), _motor(motor), hwReadbackValues{this, "extHWReadbackValues", "Variables read from the HW specfic to reference switches"}{}

void ExtHWReadbackHandler::prepare(){
  readbackFunction = std::bind(&ExtHWReadbackHandler::readback, this);
}

void ExtHWReadbackHandler::readback(){
  std::cout << "   ** ExtHWReadbackHandler::readback() called in " <<   this->_name << std::endl;
  BasicHWReadbackHandler::readback();
  hwReadbackValues.isNegativeReferenceActive = _motor->isNegativeReferenceActive();
  hwReadbackValues.isPositiveReferenceActive = _motor->isPositiveReferenceActive();
}


SWReadback::SWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor){}

void SWReadback::mainLoop(){

  std::string currentState{"NO_STATE_AVAILABLE"};

  while(true){
    // TODO Change to more efficient sampling scheme
    trigger.read();

    receiveTimer.initializeMeasurement();

    try{
    isSystemIdle = _motor->isSystemIdle();
    motorState   = _motor->getState();
    swPositionLimitsEnabled   = _motor->getSoftwareLimitsEnabled();
    maxSWPositionLimit        = _motor->getMaxPositionLimit();
    minSWPositionLimit        = _motor->getMinPositionLimit();
    maxSWPositionLimitInSteps = _motor->getMaxPositionLimitInSteps();
    minSWPositionLimitInSteps = _motor->getMinPositionLimitInSteps();

    currentLimit       = _motor->getUserCurrentLimit(); // Include when this method gets implemented
    maxCurrentLimit    = _motor->getSafeCurrentLimit(); /* TODO This is const in the library. Move to module which is run once at startup? */
    speedLimit         = _motor->getUserSpeedLimit();
    maxSpeedCapability = _motor->getMaxSpeedCapability();  /* TODO This is const in the library. Move to module which is run once at startup? */

    isFullStepping = _motor->isFullStepping();
    }
    catch(mtca4u::MotorDriverException &e){
      auto userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: ." + std::string(e.what());
      //std::cout << userMessage << std::endl;
    }


    receiveTimer.measureOnce();
    auto rt = std::chrono::duration_cast<std::chrono::microseconds>(receiveTimer.getMeasurementResult());
    actualReceiveTime = static_cast<float>(rt.count())/1000.f;

    writeAll();
  }
}
