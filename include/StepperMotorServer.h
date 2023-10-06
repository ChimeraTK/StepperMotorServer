// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ChimeraTK/MotorDriverCard/StepperMotorModule.h"
#include "MotorDummy.h"

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ApplicationCore/ConfigReader.h>
#include <ChimeraTK/ApplicationCore/PeriodicTrigger.h>

#include <memory>

static constexpr std::string_view SERVER_CONFIG_FILE{"ServerConfiguration.xml"};
static constexpr std::string_view DMAP_FILE_NAME{"devMapFile.dmap"};

namespace ctk = ChimeraTK;

struct StepperMotorServer : public ctk::Application {
  ctk::SetDMapFilePath dmapPathSetter{std::string{DMAP_FILE_NAME}};

  /********************************************************************************************************************/

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

  /********************************************************************************************************************/

  StepperMotorServer();
  ~StepperMotorServer() override { shutdown(); }

  /********************************************************************************************************************/

  ctk::ConfigReader config{this, ".", std::string{SERVER_CONFIG_FILE}};
  ctk::PeriodicTrigger trigger{this, "/Trigger", ""};

  /********************************************************************************************************************/

  std::vector<std::shared_ptr<ctk::MotorDriver::StepperMotorModule>> motorDriver;
  std::vector<MotorDummy> motorDummy;
}; /* struct StepperMotorServer */
