/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>
#include <mutex>


// Protects the StepperMotor instance shared by the modules
static std::mutex motorMutex;

// Definitions of MotorUnitsConverter class
MotorUnitConverter::MotorUnitConverter(const float userUnitToStepsRatio, const std::string &userUnit)
  : _userUnitToStepsRatio(userUnitToStepsRatio), _userUnit(userUnit){}

float MotorUnitConverter::stepsToUnits(int steps){
  return _userUnitToStepsRatio * steps;
}

int MotorUnitConverter::unitsToSteps(float units){
  return units/_userUnitToStepsRatio;
}

// TODO Is there a better way to pass Module and Motor parameters?
MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                         const MotorDriverParameters &driverParam,
                         std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter
                         )
  : ctk::ModuleGroup(owner, name, description, true),
    _motor{new ctk::StepperMotor{driverParam.motorDriverCardDeviceName,
                                 driverParam.moduleName,
                                 driverParam.motorDriverId,
                                 driverParam.motorDriverCardConfigFileName}},
   _motorUnitsConverter(unitsConverter)
{
  // TODO Prompt the library developer to provide a constructor that inits the desired converter directly
  _motor->setStepperMotorUnitsConverter(_motorUnitsConverter);
}


// Definitions of ControlInputs module
MotorDriver::ControlInput::ControlInput(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::ControlInput::prepare(){

  funcMap[enableMotor.getId()]             = [this](){ _motor->setEnabled(enableMotor); };
  funcMap[positionSetpointInSteps.getId()] = [this](){ _motor->moveToPositionInSteps(positionSetpointInSteps); };
  funcMap[stopMotor.getId()]               = [this](){ if(stopMotor){_motor->stop();}};
  funcMap[emergencyStopMotor.getId()]      = [this](){ if(emergencyStopMotor){ _motor->emergencyStop();}};
  funcMap[enableSWPositionLimits.getId()]  = [this](){ _motor->setSoftwareLimitsEnabled(enableSWPositionLimits); };
  funcMap[maxSWPositionLimit.getId()]        = [this](){ _motor->setMaxPositionLimit(maxSWPositionLimit); };
  funcMap[minSWPositionLimit.getId()]        = [this](){ _motor->setMaxPositionLimit(minSWPositionLimit); };
  funcMap[maxSWPositionLimitInSteps.getId()] = [this](){ _motor->setMaxPositionLimitInSteps(maxSWPositionLimitInSteps); };
  funcMap[minSWPositionLimitInSteps.getId()] = [this](){ _motor->setMaxPositionLimitInSteps(maxSWPositionLimitInSteps); };

}

void MotorDriver::ControlInput::mainLoop(){

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  while(true){

    auto changedVarId = inputGroup.readAny();

    {
      std::lock_guard<std::mutex> lock(motorMutex);
      if(_motor->isSystemIdle()
          || changedVarId == stopMotor.getId() || changedVarId == emergencyStopMotor.getId()){
        funcMap.at(changedVarId)();
      }
    }

    dummyMotorStop = stopMotor || emergencyStopMotor;
    dummyMotorTrigger++;

    writeAll();
  }
}



MotorDriver::HWReadback::HWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::HWReadback::mainLoop(){

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

    // Read from HW
    {
      std::lock_guard<std::mutex> lock(motorMutex);

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
    }

    writeAll();
  }
}

MotorDriver::SWReadBack::SWReadBack(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}

void MotorDriver::SWReadBack::mainLoop(){

  std::string currentState{"NO_STATE_AVAILABLE"};

  while(true){
    // TODO Change to more efficient sampling scheme
    trigger.read();

    {
      std::lock_guard<std::mutex> lock(motorMutex);

      isSystemIdle = _motor->isSystemIdle();
      motorState   = _motor->getState();
      swPositionLimitsEnabled   = _motor->getSoftwareLimitsEnabled();
      maxSWPositionLimit        = _motor->getMaxPositionLimit();
      minSWPositionLimit        = _motor->getMinPositionLimit();
      maxSWPositionLimitInSteps = _motor->getMaxPositionLimitInSteps();
      minSWPositionLimitInSteps = _motor->getMinPositionLimitInSteps();

    }

    writeAll();
  }
}
