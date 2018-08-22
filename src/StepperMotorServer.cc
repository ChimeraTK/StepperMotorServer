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


  // Setup poll trigger
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs["Timer"]("updateOnce") >> trigger.forceUpdate;
  cs["Timer"]("updateAuto") >> trigger.automaticUpdate;
  trigger.countdown >> cs["Timer"]("countdown");

  auto &cyclicTrigger = trigger.trigger;


  // Publish configuration
  //config.connectTo(cs["Configuration"]);
  /// @todo This is a work-around until string arrays do not crash the OPC UA adapter any more!
  for(auto &a : config.getAccessorList()) {
    if(a.getValueType() == typeid(std::string) && a.getNumberOfElements() > 1) {
      std::cout << "*** WARNING: Workaround active: Not publishing /Configuration/"+a.getName()+
                   " to avoid crash of OPC UA adapter." << std::endl;
      continue;
    }
    a >> cs["Configuration"](a.getName());
  }


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
    motorDriver.emplace_back(this, "MotorDriver"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), dp);
    std::cout << "*** Created motorDriver " << i << std::endl;

    motorDriver[i].controlInputs.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["controlInputs"]);
    motorDriver[i].hwReadback.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["hwReadback"]);
    //motorDriver[i].motorDriverHWReadback.findTag("MOTCTRL").connectTo(motorControl);
    motorDriver[i].swReadback.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["swReadback"]);
    //motorDriver[i].motorDriverSWReadback.findTag("MOTCTRL").connectTo(motorControl);

    if(useDummyMotors){
      motorDummy.emplace_back(this, "MotorDummy"+std::to_string(i), "Dummy for motor"+std::to_string(i), dp);
      motorDriver[i]("dummyMotorTrigger") >> motorDummy[i]("trigger");
      motorDriver[i]("dummyMotorStop") >> motorDummy[i]("stop");
    }
  }

  // Connect motorControl signals (TODO Keep this?)
  //motorControl.findTag("CS").connectTo(cs, cyclicTrigger);
  cyclicTrigger >> motorDriver[0].hwReadback("trigger");
  cyclicTrigger >> motorDriver[0].swReadback("trigger");


  // Document module structure and connections
  //motorControl.dumpGraph("motorControlModuleGraph.dot");
  motorDriver[0].dumpGraph("motorDriverModuleGraph.dot");

  dumpConnectionGraph();

} /* defineConnections() */

static StepperMotorServer server;

