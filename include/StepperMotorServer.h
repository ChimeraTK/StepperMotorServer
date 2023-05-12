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

static constexpr std::string_view SERVER_CONFIG_FILE{"ServerConfiguration.xml"};
static constexpr std::string_view DMAP_FILE_NAME{"devMapFile.dmap"};

namespace ctk = ChimeraTK;

struct StepperMotorServer : public ctk::Application {
  ctk::SetDMapFilePath dmapPathSetter{std::string{DMAP_FILE_NAME}};

  StepperMotorServer();
  ~StepperMotorServer() override { shutdown(); }

  ctk::ConfigReader config{this, ".", std::string{SERVER_CONFIG_FILE}};

  struct : ctk::ApplicationModule {
    using ctk::ApplicationModule::ApplicationModule;

    ctk::ScalarPushInput<uint64_t> trigger{this, "/Trigger/tick", "", ""};
    ctk::VoidOutput tick{this, "/Trigger/voidTick", ""};

    void mainLoop() final {
      do {
        tick.write();
        trigger.read();
      } while(true);
    }

  } triggerConverter{this, ".", ""};

  ctk::PeriodicTrigger trigger{this, "/Trigger", ""};

  std::vector<ctk::MotorDriver::StepperMotorModule> motorDriver;
  std::vector<MotorDummy> motorDummy;
};     /* struct StepperMotorServer */
#endif /* INCLUDE_STEPPER_MOTOR_SERVER_H_ */
