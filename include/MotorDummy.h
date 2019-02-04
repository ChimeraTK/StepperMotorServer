/*
 * MotorDummy.h
 *
 *  Created on: Aug 14, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORDUMMY_H_
#define INCLUDE_MOTORDUMMY_H_

#include <ChimeraTK/ApplicationCore/Application.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <ChimeraTK/ApplicationCore/ApplicationModule.h>

#include "mtca4u/MotorDriverCard/MotorDriverCardFactory.h"
#include "mtca4u/MotorDriverCard/MotorControlerDummy.h"
#include "mtca4u/MotorDriverCard/StepperMotor.h"

#include <boost/shared_ptr.hpp>
#include <thread>

namespace ctk = ChimeraTK;
namespace ctkmot = ctk::MotorDriver;


/** MotorDummy - A dummy application module to employ the MotorControlerDummy functionality of the MotorDriverCard library
 *
 * Note: This is a workaround to handle the dummy provided by the MotorDriverCard library, which requires specific function calls
 *       not supported by the real MotorControler.
 */
struct MotorDummy : public ctk::ApplicationModule {

  MotorDummy(ctk::EntityOwner *owner, const std::string &name, const std::string &description,const ctkmot::StepperMotorParameters &dp)
    : ctk::ApplicationModule(owner, name, description, true),
      _motorControlerDummy(boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(
          mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(dp.deviceName, dp.moduleName, dp.configFileName)->getMotorControler(dp.driverId)))
      {};

  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;


  ctk::ScalarPushInput<int32_t> trigger{this, "dummyMotorTrigger", "", "Triggers movement of the dummy motor"};
  ctk::ScalarPollInput<int32_t> stop{this, "dummyMotorStop","", "Stops the dummy motor"};

  void mainLoop(){

    // FIXME HACK: enabled this permanently because there is no relation between StepperMotor and the dummy of the library
    //             and I don't want to reprogram it here
    _motorControlerDummy->setMotorCurrentEnabled(true);
    _motorControlerDummy->setEndSwitchPowerEnabled(true);

    while(true){
      trigger.read();

      while(!_motorControlerDummy->isMotorMoving() &&
            _motorControlerDummy->getTargetPosition() != _motorControlerDummy->getActualPosition()){
       std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

      float moveFraction = .2f;
      while(_motorControlerDummy->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        stop.read();
        if(!stop){
          _motorControlerDummy->moveTowardsTarget(moveFraction);
          moveFraction *= 1.5f; // Quick hack so the dummy actually reaches its target
        }
        else{
          continue;
        }
      }
    }
  }
};

#endif /* INCLUDE_MOTORDUMMY_H_ */
