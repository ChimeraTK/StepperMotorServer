/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>


// FIXME Motor specs must be read from config
MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ModuleGroup(owner, name, description, true), _motor{new ctk::StepperMotor{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"}}{}

// FIXME Change to delegate ctor once we use the prepare method
MotorDriver::ControlInputs::ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {

}


void MotorDriver::ControlInputs::prepare(){

  inputGroup = this->readAnyGroup();

  funcMap[enableMotor.getId()] = [this](){_motor->setEnabled(enableMotor); std::cout << "****** Called setEnabled()" << std::endl;};
  funcMap[positionSetpoint.getId()] = [this](){_motor->moveToPosition(positionSetpoint);  std::cout << "****** Called moveToPostion()" << std::endl;};
  funcMap[stopMotor.getId()] = [this](){_motor->stop(); std::cout << "****** Called stop()" << std::endl;};
  funcMap[emergencyStopMotor.getId()] = [this](){_motor->emergencyStop(); std::cout << "****** Called emergencyStop()" << std::endl;};

}

void MotorDriver::ControlInputs::mainLoop(){

  // Initialize the motor
  _motor->setActualPositionInSteps(0);


  while(true){

    auto changedVarId = inputGroup.readAny();
    funcMap.at(changedVarId)();

    std::cout << "Motor use count in ControlInputs instance: " << _motor.use_count() << std::endl;
//    _motor->setEnabled(true);
//    _motor->setActualPosition(42.1234f);
    actualPosition = .0;

    writeAll();
  }
}


MotorDriver::MotorDriverHWReadback::MotorDriverHWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {}


void MotorDriver::MotorDriverHWReadback::mainLoop(){

//  ctk::AccessModeFlags amf = trigger.getAccessModeFlags();
//
//  if(amf == ctk::AccessMode::wait_for_new_data){
//    std::cout << "Access mode is 'wait_for_new_data'" << std::endl;
//  }
//  else{
//    std::cout << "Wrong access mode for trigger in MotorDriverReadback." << std::endl;
//  }

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

  while(true){
    // TODO Change to more efficient sampling scheme
    trigger.read();
    isSystemIdle = _motor->isSystemIdle() ? 1 : 0;

    writeAll();
  }
}
