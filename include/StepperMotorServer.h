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

namespace ctk = ChimeraTK;

struct StepperMotorServer : ctk::Application{
  StepperMotorServer() : Application("StepperMotorServer") {}
  ~StepperMotorServer() { shutdown(); }

  ctk::ConfigReader config{this, "Configuration", "CTK_StepperMotorConfig.xml"};

  Trigger trigger{this, "TRIGGER", ""};
  Timer timer{this, "TIMER", ""};

  ctk::ControlSystemModule cs{"Motor"};

  MotorControl motorControl;

  void defineConnections() override;

}; /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
