// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "mtca4u/MotorDriverCard/MotorControlerDummy.h"

#include <ChimeraTK/ApplicationCore/Application.h>
#include <ChimeraTK/ApplicationCore/ApplicationModule.h>
#include <ChimeraTK/ApplicationCore/ScalarAccessor.h>
#include <ChimeraTK/MotorDriverCard/StepperMotor.h>
#include <ChimeraTK/ReadAnyGroup.h>

#include <boost/shared_ptr.hpp>

#include <thread>

namespace ctk = ChimeraTK;
namespace ctkmot = ChimeraTK::MotorDriver;

/**
 *
 * MotorDummy - A dummy application module to employ the MotorControlerDummy functionality of the MotorDriverCard
 * library
 *
 * Note: This is a workaround to handle the dummy provided by the MotorDriverCard library, which requires specific
 * function calls not supported by the real MotorControler.
 */
struct MotorDummy : public ctk::ApplicationModule {
  MotorDummy(ctk::ModuleGroup* owner, const std::string& name, const std::string& description,
      const ctkmot::StepperMotorParameters& dp);

  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;

  ctk::ScalarPushInput<int32_t> trigger{
      this, "controlInput/dummySignals/dummyMotorTrigger", "", "Triggers movement of the dummy motor"};
  ctk::ScalarPollInput<int32_t> stop{this, "controlInput/dummySignals/dummyMotorStop", "", "Stops the dummy motor"};

  void mainLoop() override;
};
