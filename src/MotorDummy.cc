// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MotorDummy.h"

#include <mtca4u/MotorDriverCard/MotorDriverCard.h>
#include <mtca4u/MotorDriverCard/MotorDriverCardFactory.h>

/********************************************************************************************************************/

MotorDummy::MotorDummy(ChimeraTK::ModuleGroup* owner, const std::string& name, const std::string& description,
    const ChimeraTK::MotorDriver::StepperMotorParameters& dp)
: ctk::ApplicationModule(owner, name, description),
  _motorControlerDummy(
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance()
              .createMotorDriverCard(dp.deviceName, dp.moduleName, dp.configFileName)
              ->getMotorControler(dp.driverId))) {}

/********************************************************************************************************************/

void MotorDummy::mainLoop() {
  // FIXME HACK: enabled this permanently because there is no relation between StepperMotor and the dummy of the library
  //             and I don't want to reprogram it here
  _motorControlerDummy->setMotorCurrentEnabled(true);
  _motorControlerDummy->setEndSwitchPowerEnabled(true);

  while(true) {
    trigger.read();

    while(!_motorControlerDummy->isMotorMoving() &&
        _motorControlerDummy->getTargetPosition() != _motorControlerDummy->getActualPosition()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    float moveFraction = .2f;
    while(_motorControlerDummy->isMotorMoving()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      stop.read();
      if(!stop) {
        _motorControlerDummy->moveTowardsTarget(moveFraction);
        moveFraction *= 1.5F; // Quick hack so the dummy actually reaches its target
      }
      else {
        continue;
      }
    }
  }
}
