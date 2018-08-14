/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>


// TODO Is there a better way to pass Module and Motor parameters?
MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description, MotorDriverParameters driverParam)
  : ctk::ModuleGroup(owner, name, description, true),
    _motor{new ctk::StepperMotor{driverParam.motorDriverCardDeviceName,
                                 driverParam.moduleName,
                                 driverParam.motorDriverId,
                                 driverParam.motorDriverCardConfigFileName}}{}

// FIXME Change to delegate ctor once we use the prepare method
MotorDriver::ControlInputs::ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::ControlInputs::prepare(){

  inputGroup = this->readAnyGroup();

  funcMap[enableMotor.getId()] = [this](){_motor->setEnabled(enableMotor); std::cout << "****** Called setEnabled()" << std::endl;};
  funcMap[positionSetpoint.getId()] = [this](){_motor->moveToPosition(positionSetpoint);  std::cout << "****** Called moveToPostion()" << std::endl;};
  funcMap[stopMotor.getId()] = [this](){_motor->stop(); std::cout << "****** Called stop()" << std::endl;};
  funcMap[emergencyStopMotor.getId()] = [this](){_motor->emergencyStop(); std::cout << "****** Called emergencyStop()" << std::endl;};

}

void MotorDriver::ControlInputs::mainLoop(){

  std::cout << "Entered controlInputs mainLoop. Motor state: " << _motor->isSystemIdle() << "\n" << std::endl;

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  std::cout << "Motor state: " << _motor->isCalibrated() << "\n" << std::endl;


  while(true){

    auto changedVarId = inputGroup.readAny();
    if(_motor->isSystemIdle()){
      funcMap.at(changedVarId)();
    }

    actualPosition = .0;

    writeAll();
  }
}


MotorDriver::MotorDriverHWReadback::MotorDriverHWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::MotorDriverHWReadback::mainLoop(){

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

MotorDriver::MotorDriverSWReadBack::MotorDriverSWReadBack(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}

void MotorDriver::MotorDriverSWReadBack::mainLoop(){

  std::string currentState = "";

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
