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

#include "ChimeraTK/MotorDriverCard/StepperMotorModule.h"
#include "MotorDummy.h"

#include <memory>

static std::string serverConfigFile{"ServerConfiguration.xml"};

namespace ctk = ChimeraTK;

struct StepperMotorServer : public ctk::Application {
  StepperMotorServer() : Application("steppermotorserver") {}
  ~StepperMotorServer() override { shutdown(); }

  ctk::ConfigReader config{this, "Configuration", serverConfigFile};

  ctk::PeriodicTrigger trigger{this, "trigger", ""};

  std::vector<ctk::MotorDriver::StepperMotorModule> motorDriver;
  std::vector<MotorDummy> motorDummy;

  ctk::ControlSystemModule cs;
  void defineConnections() override;

};     /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
