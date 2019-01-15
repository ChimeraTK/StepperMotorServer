/*
 * MotorDriverReadback.cc
 *
 *  Created on: Sep 17, 2018
 *      Author: ckampm
 */

#include "MotorDriverReadback.h"

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

    //readbackFunction();
    readback();
    if(_motor->hasHWReferenceSwitches()){
      readEndSwitchData();
    }

    receiveTimer.measureOnce();
    auto rt = std::chrono::duration_cast<std::chrono::microseconds>(receiveTimer.getMeasurementResult());
    actualReceiveTime = static_cast<float>(rt.count())/1000.f;

    writeAll();
  }
}


ReadbackHandler::ReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor,
                                               ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule::ApplicationModule(owner, name, description),
    hwReadbackValues{this, "hwReadbackValues", "Variables read from the HW of the MotorDriver", true},
    swReadbackValues{this, "swReadbackValues", "Variables read from the SW of the MotorDriver", true},
    extHWReadbackValues{},
    positiveEndSwitch{},
    negativeEndSwitch{},
    _motor{motor}
{
  if(_motor->hasHWReferenceSwitches()){
    extHWReadbackValues = HWReadbackValuesExt{this, "extHWReadbackValues", "Variables read from the HW specific to reference switches", true};
    positiveEndSwitch   = ReferenceSwitch{this, "positiveEndSwitch", "Data of the positive end switch", /*true}*/};
    negativeEndSwitch   = ReferenceSwitch{this, "negativeEndSwitch", "Data of the negative end switch", /*true*/};
  }
}

//void BasicHWReadbackHandler::prepare(){
//  readbackFunction = std::bind(&BasicHWReadbackHandler::readback, this);
//}

void ReadbackHandler::readback(){
    // Read from HW
    hwReadbackValues.isCalibrated = _motor->isCalibrated() ? 1 : 0;
    ctk::StepperMotorError error = _motor->getError();
    hwReadbackValues.motorErrorId = static_cast<int32_t>(error);
    hwReadbackValues.actualPositionInSteps = _motor->getCurrentPositionInSteps();
    hwReadbackValues.encoderPosition= _motor->getEncoderPosition();

    // Update values that have a static relation to HW readback
    hwReadbackValues.actualPosition = _motor->recalculateStepsInUnits(hwReadbackValues.actualPositionInSteps);

    // FIXME Debug RBVs
    hwReadbackValues.enabledRBV = _motor->getEnabled();
    hwReadbackValues.targetPositionInStepsRBV = _motor->getTargetPositionInSteps();

    try{
      swReadbackValues.isSystemIdle              = _motor->isSystemIdle();
      swReadbackValues.motorState                = _motor->getState();
      swReadbackValues.swPositionLimitsEnabled   = _motor->getSoftwareLimitsEnabled();
      swReadbackValues.maxSWPositionLimit        = _motor->getMaxPositionLimit();
      swReadbackValues.minSWPositionLimit        = _motor->getMinPositionLimit();
      swReadbackValues.maxSWPositionLimitInSteps = _motor->getMaxPositionLimitInSteps();
      swReadbackValues.minSWPositionLimitInSteps = _motor->getMinPositionLimitInSteps();

      swReadbackValues.currentLimit       = _motor->getUserCurrentLimit(); // Include when this method gets implemented
      swReadbackValues.maxCurrentLimit    = _motor->getSafeCurrentLimit(); /* TODO This is const in the library. Move to module which is run once at startup? */
      swReadbackValues.speedLimit         = _motor->getUserSpeedLimit();
      swReadbackValues.maxSpeedCapability = _motor->getMaxSpeedCapability();  /* TODO This is const in the library. Move to module which is run once at startup? */

      swReadbackValues.isFullStepping   = _motor->isFullStepping();
      swReadbackValues.autostartEnabled = _motor->getAutostart();
    }
    catch(mtca4u::MotorDriverException &e){
      auto userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: ." + std::string(e.what());
    }

}


//ExtHWReadbackHandler::ExtHWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor,
//                                            ctk::EntityOwner *owner, const std::string &name, const std::string &description)
//  : BasicHWReadbackHandler(motor, owner, name, description),
//    _motor(motor),
//    hwReadbackValues{this, "extHWReadbackValues", "Variables read from the HW specific to reference switches", true},
//    swReadbackValues{this, "extSWReadbackValues", "Variables read from the SW of the MotorDriver specific to reference switches", true} {}

//void ReadbackHandler::prepare(){
//  readbackFunction = std::bind(&ExtHWReadbackHandler::readback, this);
//}

void ReadbackHandler::readEndSwitchData(){

  extHWReadbackValues.isNegativeReferenceActive = _motor->isNegativeReferenceActive();
  extHWReadbackValues.isPositiveReferenceActive = _motor->isPositiveReferenceActive();

  positiveEndSwitch.position                 = _motor->getPositiveEndReference();
  negativeEndSwitch.position                 = _motor->getNegativeEndReference();
  positiveEndSwitch.positionInSteps          = _motor->getPositiveEndReferenceInSteps();
  negativeEndSwitch.positionInSteps          = _motor->getNegativeEndReferenceInSteps();

  positiveEndSwitch.tolerance = _motor->getTolerancePositiveEndSwitch();
  negativeEndSwitch.tolerance = _motor->getToleranceNegativeEndSwitch();
}


//BasicSWReadbackHandler::BasicSWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
//  : ReadbackHandler(owner, name, description),
//    _motor(motor),
//    swReadbackValues{this, "swReadbackValues", "Variables read from the SW of the MotorDriver", true} {}
//
//void BasicSWReadbackHandler::prepare(){
//  readbackFunction = std::bind(&BasicSWReadbackHandler::readback, this);
//}
//
//void BasicSWReadbackHandler::readback(){
//
//  try{
//    swReadbackValues.isSystemIdle              = _motor->isSystemIdle();
//    swReadbackValues.motorState                = _motor->getState();
//    swReadbackValues.swPositionLimitsEnabled   = _motor->getSoftwareLimitsEnabled();
//    swReadbackValues.maxSWPositionLimit        = _motor->getMaxPositionLimit();
//    swReadbackValues.minSWPositionLimit        = _motor->getMinPositionLimit();
//    swReadbackValues.maxSWPositionLimitInSteps = _motor->getMaxPositionLimitInSteps();
//    swReadbackValues.minSWPositionLimitInSteps = _motor->getMinPositionLimitInSteps();
//
//    swReadbackValues.currentLimit       = _motor->getUserCurrentLimit(); // Include when this method gets implemented
//    swReadbackValues.maxCurrentLimit    = _motor->getSafeCurrentLimit(); /* TODO This is const in the library. Move to module which is run once at startup? */
//    swReadbackValues.speedLimit         = _motor->getUserSpeedLimit();
//    swReadbackValues.maxSpeedCapability = _motor->getMaxSpeedCapability();  /* TODO This is const in the library. Move to module which is run once at startup? */
//
//    swReadbackValues.isFullStepping   = _motor->isFullStepping();
//    swReadbackValues.autostartEnabled = _motor->getAutostart();
//  }
//  catch(mtca4u::MotorDriverException &e){
//    auto userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: ." + std::string(e.what());
//  }
//}
//
//ExtSWReadbackHandler::ExtSWReadbackHandler(std::shared_ptr<ctk::StepperMotorWithReference> motor,
//                                            ctk::EntityOwner *owner, const std::string &name, const std::string &description)
//  : BasicSWReadbackHandler(motor, owner, name, description),
//    _motor(motor),
//    swReadbackValues{this, "extSWReadbackValues", "Variables read from the SW of the MotorDriver specfic to reference switches", true}{}
//
//void ExtSWReadbackHandler::prepare(){
//  readbackFunction = std::bind(&ExtSWReadbackHandler::readback, this);
//}
//
//void ExtSWReadbackHandler::readback(){
//  BasicSWReadbackHandler::readback();
//  swReadbackValues.positiveEndReference        = _motor->getPositiveEndReference();
//  swReadbackValues.negativeEndReference        = _motor->getNegativeEndReference();
//  swReadbackValues.positiveEndReferenceInSteps = _motor->getPositiveEndReferenceInSteps();
//  swReadbackValues.negativeEndReferenceInSteps = _motor->getNegativeEndReferenceInSteps();
//
//  swReadbackValues.positiveEndReferenceTolerance = _motor->getTolerancePositiveEndSwitch();
//  swReadbackValues.negativeEndReferenceTolerance = _motor->getToleranceNegativeEndSwitch();
//}

