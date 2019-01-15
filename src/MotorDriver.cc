/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <memory>




BasicMotorDriver::BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                   ctk::StepperMotorParameters &motorParameters)
  : ctk::ModuleGroup(owner, name, description),
  motor{ctk::StepperMotorFactory::instance().create(ctk::StepperMotorType::BASIC, std::move(motorParameters))},
  ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", motor},
  readbackHandler{motor, this, "hwReadback", "Signals read from the motor driver"}{}


LinearMotorDriver::LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                     ctk::StepperMotorParameters &motorParameters)
  : ctk::ModuleGroup(owner, name, description),
    motor{ctk::StepperMotorFactory::instance().create(ctk::StepperMotorType::LINEAR, std::move(motorParameters))},
    ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", motor},
    readbackHandler{motor, this, "hwReadback", "Signals read from the motor driver"}{}
