/*
 * StepperMotorServer.h
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_STEPPER_MOTOR_SERVER_H_
#define INCLUDE_STEPPER_MOTOR_SERVER_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ApplicationCore/ConfigReader.h>

#include <ChimeraTK/Device.h>

#include "MotorControl.h"
#include "Trigger.h"

#include <vector>
#include <memory>

namespace ctk = ChimeraTK;

struct StepperMotorServer : ctk::Application{
  StepperMotorServer() : Application("CTK_StepperMotorServer") {}
  ~StepperMotorServer() { shutdown(); }

  ctk::ConfigReader config{this, "Configuration", "CTK_StepperMotorConfig.xml"};

  Trigger trigger{this, "TRIGGER", ""};
  Timer<> timer{this, "TIMER", ""};

  ctk::ControlSystemModule cs{"MOTOR1"};

  MotorControl motorControl{this, "MotorControl", ""};

  //ctk::DeviceModule dm{"BeckhoffMotor"};
  std::vector< std::shared_ptr<ctk::Module> > motor;

  void defineConnections() override;

}; /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
