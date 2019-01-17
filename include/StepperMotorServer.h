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
#include <ChimeraTK/ApplicationCore/PeriodicTrigger.h>

#include "MotorDriver.h"
#include "MotorDummy.h"
//#include "Trigger.h"

#include <memory>

static std::string serverConfigFile{"CTK_StepperMotorConfig.xml"};

namespace ctk = ChimeraTK;

struct StepperMotorServer : ctk::Application{
  StepperMotorServer() : Application("CTK_StepperMotorServer") {}
  //~StepperMotorServer() override { shutdown(); }

  ctk::ConfigReader config{this, "Configuration", serverConfigFile};

  //Trigger trigger{this, "trigger", ""};
  //Timer<std::chrono::milliseconds> timer{this, "TIMER", ""};
  ctk::PeriodicTrigger trig{this, "trigger", ""};

  std::vector<MotorDriver> motorDriver;
  std::vector<MotorDummy> motorDummy;

  ctk::ControlSystemModule cs;
  void defineConnections() override;

}; /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
