/*
 * StepperMotorServer.cc
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#include "StepperMotorServer.h"

#include "version.h"
#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>
#include "mtca4u/MotorDriverCard/MotorDriverCardFactory.h"
#include "mtca4u/DMapFilesParser.h"

#include <boost/shared_ptr.hpp>

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

  // TODO Handle mix of real and dummy motors in dMap file
  bool useDummyMotors = false;
  if(ctk::DMapFilesParser(".").getdMapFileElem(stepperMotorDeviceName).uri == "/dummy/MotorDriverCard"){

    useDummyMotors = true;
    mtca4u::MotorDriverCardFactory::instance().setDummyMode();
    std::cout << "*** Dummy motor in use." << std::endl;
  }

  // Set up modules for each motor
  auto nMotors = config.get<uint32_t>("nMotors");
  for(size_t i = 0; i<nMotors; ++i){

    //FIXME Read from config and handle multiple motors
    MotorDriverParameters dp{"MOTOR_DUMMY", "MD22.1", 1U, "motorConfig.xml"};
    motorDriver.emplace_back(this, "MotorDriver"+std::to_string(i), "Driver of motor "+std::to_string(i), dp);
    std::cout << "*** Created motorDriver " << i << std::endl;

    motorControl.toMotorDriver.connectTo(motorDriver[i]);
    motorDriver[i].motorDriverHWReadback.findTag("CS").connectTo(cs["motorDriverReadback"]);
    motorDriver[i].motorDriverHWReadback.findTag("MOTCTRL").connectTo(motorControl);
    motorDriver[i].motorDriverSWReadback.findTag("CS").connectTo(cs["motorDriverReadback"]);
    motorDriver[i].motorDriverSWReadback.findTag("MOTCTRL").connectTo(motorControl);
    //motorDriver[i].motorDriverSWReadback.isSystemIdle >> motorDriver[i].controlInputs.systemIdle;

    if(useDummyMotors){
      motorDummy.emplace_back(this, "MotorDummy"+std::to_string(i), "Dummy for motor"+std::to_string(i), dp);
      motorDriver[i]("dummyMotorTrigger") >> motorDummy[i]("trigger");
    }
  }


  //TODO Current wiring of the trigger and other push-type inputs
  //     leads to updates inbetween samples. Rearrange triggers to have a
  //     'hard' cycle time

  // Setup poll trigger
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs("TIMER.UPDATE.ONCE") >> trigger.forceUpdate;
  cs("TIMER.UPDATE.AUTO") >> trigger.automaticUpdate;
  trigger.countdown >> cs("TIMER.COUNTDOWN");

  auto &triggerNr = trigger.trigger;

  // Connect motorControl signals (TODO Keep this?)
  motorControl.findTag("CS").connectTo(cs, triggerNr);
  triggerNr >> motorDriver[0].motorDriverHWReadback("trigger");
  triggerNr >> motorDriver[0].motorDriverSWReadback("trigger");


  // Document module structure and connections
  motorControl.dumpGraph("motorControlModuleGraph.dot");
  motorDriver[0].dumpGraph("motorDriverModuleGraph.dot");

  dumpConnectionGraph();

} /* defineConnections() */

static StepperMotorServer server;

