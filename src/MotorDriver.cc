/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>



MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ModuleGroup(owner, name, description, true), _motor{new ctk::StepperMotor{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"}}{

}

// FIXME Change to delegate ctor once we use the prepare method
MotorDriver::ControlInputs::ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor(motor) {

}


void MotorDriver::ControlInputs::prepare(){
  funcMap[enableMotor.getId()] = [this](){_motor->setEnabled(enableMotor); std::cout << "****** Called setEnabled()" << std::endl;};
  funcMap[positionSetpoint.getId()] = [this](){_motor->moveToPosition(positionSetpoint);  std::cout << "****** Called moveToPostion()" << std::endl;};
  funcMap[stopMotor.getId()] = [this](){_motor->stop();};
  funcMap[emergencyStopMotor.getId()] = [this](){_motor->emergencyStop();};

}

void MotorDriver::ControlInputs::mainLoop(){

  while(true){
    auto id = readAny();

    funcMap[id]();

    std::cout << "Motor use count in ControlInputs instance: " << _motor.use_count() << std::endl;
//    _motor->setEnabled(true);
//    _motor->setActualPosition(42.1234f);
    actualPosition = (double)_motor->getCurrentPosition();
    actualPosition.write();
  }
}
