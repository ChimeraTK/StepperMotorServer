/*
 * StepperMotorServer.h
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_STEPPER_MOTOR_SERVER_H_
#define INCLUDE_STEPPER_MOTOR_SERVER_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/Device.h>

#include "MotorControl.h"
#include "Trigger.h"

namespace ctk = ChimeraTK;

struct StepperMotorServer : ctk::Application{

  ctk::ControlSystemModule cs;

  MotorControl motorControl;

  std::vector<ctk::DeviceModule> motors;


  void defineConnections();

} /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
