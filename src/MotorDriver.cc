/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>


// TODO Is there a better way to pass Module and Motor parameters?
MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description, const MotorDriverParameters &driverParam)
  : ctk::ModuleGroup(owner, name, description, true),
    _motor{new ctk::StepperMotor{driverParam.motorDriverCardDeviceName,
                                 driverParam.moduleName,
                                 driverParam.motorDriverId,
                                 driverParam.motorDriverCardConfigFileName}}{}


// Definitions of ControlInputs module
MotorDriver::ControlInputs::ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::ControlInputs::prepare(){

  funcMap[enableMotor.getId()]             = [this](){ _motor->setEnabled(enableMotor); };
  funcMap[positionSetpointInSteps.getId()] = [this](){ _motor->moveToPositionInSteps(positionSetpointInSteps); };
  funcMap[stopMotor.getId()]               = [this](){ if(stopMotor){_motor->stop();}};
  funcMap[emergencyStopMotor.getId()]      = [this](){ if(emergencyStopMotor){ _motor->emergencyStop();}};

}

void MotorDriver::ControlInputs::mainLoop(){

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  while(true){

    auto changedVarId = inputGroup.readAny();

    if(_motor->isSystemIdle()
        || changedVarId == stopMotor.getId() || changedVarId == emergencyStopMotor.getId()){
      funcMap.at(changedVarId)();
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
    isCalibrated = _motor->isCalibrated() ? 1 : 0;
    ctk::StepperMotorError error = _motor->getError();
    motorErrorId = static_cast<int32_t>(error);
    actualPositionInSteps = _motor->getCurrentPositionInSteps();

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

    isSystemIdle = _motor->isSystemIdle();

    if(_motor->getState() != currentState){
      currentState = _motor->getState();
      std::cout << "**** New motorDriver state: " << currentState << std::endl;
    }


    writeAll();
  }
}
