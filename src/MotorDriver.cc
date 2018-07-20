/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>



MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ModuleGroup(owner, name, description, true){

  //_motor = std::make_shared<ctk::StepperMotor>("MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml");
  //_motor = std::shared_ptr<ctk::StepperMotor>{new ctk::StepperMotor{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"}};
  //_motor = ctk::StepperMotor{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"};
}

// FIXME Change to delegate ctor once we use the prepare method
MotorDriver::ControlInputs::ControlInputs(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true), _motor{new ctk::StepperMotor{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"}} {

}


void MotorDriver::ControlInputs::prepare(){
  funcMap[emergencyStopMotor.getId()] = [this](){_motor->emergencyStop();};
  //funcMap.insert(positionSetpoint.getId, [](){motor.moveToPosition(static_cast<float>(positionSetpoint.read()));});
  //funcMap[stopMotor.getId()] = [](){motor.stop();};
}

void MotorDriver::ControlInputs::mainLoop(){

  while(true){
    auto id = readAny();

//    if (id == emergencyStopMotor.getId()){
//     motor.emergencyStop();
//    }
//    funcMap[id]();

    std::cout << "Motor use count in ControlInputs instance: " << _motor.use_count() << std::endl;
    _motor->setEnabled(true);
    _motor->setActualPosition(42.1234f);
    actualPosition = (double)_motor->getCurrentPosition();
    actualPosition.write();
  }
}
