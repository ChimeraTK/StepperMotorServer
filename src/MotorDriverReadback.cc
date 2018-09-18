/*
 * MotorDriverReadback.cc
 *
 *  Created on: Sep 17, 2018
 *      Author: ckampm
 */

#include "MotorDriverReadback.h"


HWReadback::HWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor){}


void HWReadback::mainLoop(){

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


    // Read from HW
    isCalibrated = _motor->isCalibrated() ? 1 : 0;
    ctk::StepperMotorError error = _motor->getError();
    motorErrorId = static_cast<int32_t>(error);
    actualPositionInSteps = _motor->getCurrentPositionInSteps();
    decoderPosition = _motor->getDecoderPosition();

    // Update values that have a static relation to HW readback
    actualPosition = _motor->recalculateStepsInUnits(actualPositionInSteps);

    // FIXME Debug RBVs
    enabledRBV = _motor->getEnabled();
    targetPositionInStepsRBV = _motor->getTargetPositionInSteps();


    receiveTimer.measureOnce();
    auto rt = std::chrono::duration_cast<std::chrono::microseconds>(receiveTimer.getMeasurementResult());
    actualReceiveTime = static_cast<float>(rt.count())/1000.f;

    writeAll();
  }
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
