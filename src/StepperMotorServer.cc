/*
 * StepperMotorServer.cc
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#include "StepperMotorServer.h"

#include "version.h"
#include "mtca4u/MotorDriverCard/MotorDriverCardFactory.h"
#include "mtca4u/DMapFilesParser.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>


// FIXME Get these from the CS
static const std::string stepperMotorDeviceName("MOTOR_DUMMY");


void StepperMotorServer::defineConnections(){

  std::string dMapFileName{"dummies.dmap"};

  mtca4u::setDMapFilePath(dMapFileName);

  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor server version "
            << AppVersion::major << "." << AppVersion::minor << "." << AppVersion::patch << std::endl;

  if(ctk::DMapFilesParser(".").getdMapFileElem(stepperMotorDeviceName).uri == "/dummy/MotorDriverCard"){
    mtca4u::MotorDriverCardFactory::instance().setDummyMode();
    std::cout << "*** Dummy motor in use." << std::endl;
  }


  auto nMotors = config.get<uint32_t>("nMotors");
  for(size_t i = 0; i<nMotors; ++i){
    motorDriver.emplace_back(this, "MotorDriver"+std::to_string(i), "Driver of motor "+std::to_string(i));
    std::cout << "*** Created motorDriver " << i << std::endl;
    motorDriver[i].findTag("CTRL").connectTo(motorControl);
    motorDriver[i]("ACT_POS") >> motorControl("CURPOS");
  }


  //TODO Current wiring of the trigger and other push-type inputs
  //     leads to updates inbetween samples. Rearrange triggers to have a
  //     'hard' cycle time

  // Setup poll trigger
  //auto cycleTime = config.get<int32_t>("cycleTime");
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs("TIMER.UPDATE.ONCE") >> trigger.forceUpdate;
  cs("TIMER.UPDATE.AUTO") >> trigger.automaticUpdate;
  trigger.countdown >> cs("TIMER.COUNTDOWN");

  auto &triggerNr = trigger.trigger;

  // Connect motorControl signals
  motorControl.findTag("CS").connectTo(cs, triggerNr);


} /* defineConnections() */
