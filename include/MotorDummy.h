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

#include <boost/shared_ptr.hpp>

namespace ctk = ChimeraTK;

//FIXME This is duplicated from MotorDriver
#include "MotorDriverParameters.h"


/** MotorDummy - A dummy application module to employ the MotorControlerDummy functionality of the MotorDriverCard library */
struct MotorDummy : public ctk::ApplicationModule {

  MotorDummy(ctk::EntityOwner *owner, const std::string &name, const std::string &description, MotorDriverParameters dp)
    : ctk::ApplicationModule(owner, name, description, true), _dp(dp){};

    MotorDriverParameters _dp;
    boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;


  ctk::ScalarPushInput<int32_t> trigger{this, "trigger", "", "Triggers one execution step"};

  void mainLoop(){

  mtca4u::MotorDriverCardFactory& factory{mtca4u::MotorDriverCardFactory::instance()};
  // FIXME Here we get bad alloc
  boost::shared_ptr<mtca4u::MotorDriverCard> motorDriverCard{factory.createMotorDriverCard(_dp.motorDriverCardDeviceName, _dp.moduleName, _dp.motorDriverCardConfigFileName)};
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(motorDriverCard->getMotorControler(0));

//  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(
//      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(_dp.motorDriverCardDeviceName, _dp.moduleName, _dp.motorDriverCardConfigFileName)->getMotorControler(0));

    while(true){
      trigger.read();
      _motorControlerDummy->moveTowardsTarget(1.f);
    }
  }
};

#endif /* INCLUDE_MOTORDUMMY_H_ */
