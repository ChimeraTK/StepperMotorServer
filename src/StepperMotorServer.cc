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
#include <algorithm>



void StepperMotorServer::defineConnections(){

  std::string dMapFileName{"dummies.dmap"};

  mtca4u::setDMapFilePath(dMapFileName);

  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor Server version "
            << AppVersion::major << "." << AppVersion::minor << "." << AppVersion::patch << std::endl;


  // Setup poll trigger
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs["Timer"]("updateOnce") >> trigger.forceUpdate;
  cs["Timer"]("updateAuto") >> trigger.automaticUpdate;
  trigger.countdown >> cs["Timer"]("countdown");

  auto &cyclicTrigger = trigger.trigger;


  // Publish configuration
  config.connectTo(cs["Configuration"]);
  /// @todo This is a work-around until string arrays do not crash the OPC UA adapter any more!
//  for(auto &a : config.getAccessorList()) {
//    if(a.getValueType() == typeid(std::string) && a.getNumberOfElements() > 1) {
//      std::cout << "*** WARNING: Workaround active: Not publishing /Configuration/"+a.getName()+
//                   " to avoid crash of OPC UA adapter." << std::endl;
//      continue;
//    }
//    a >> cs["Configuration"](a.getName());
//  }

  // Get and validate motor configuration
  auto nMotors = config.get<uint32_t>("nMotors");
  std::vector<std::string> motorDriverCardDeviceNames = config.get<std::vector<std::string>>("motorDriverDeviceName");
  std::vector<std::string> motorDriverCardModuleNames = config.get<std::vector<std::string>>("motorDriverModuleName");
  std::vector<uint32_t> motorDriverCardIds            = config.get<std::vector<uint32_t>>("motorDriverId");
  std::vector<std::string> motorDriverCardConfigFiles = config.get<std::vector<std::string>>("motorDriverConfigFile");

  if(motorDriverCardDeviceNames.size() != nMotors ||
     motorDriverCardModuleNames.size() != nMotors ||
     motorDriverCardIds.size()         != nMotors ||
     motorDriverCardConfigFiles.size() != nMotors)
  {
    std::cout << "!!! Terminating the server!" << std::endl
             << "A mismatch of dimensions in the MotorDriver configuration provided by" << serverConfigFile
             << "has been detected" << std::endl;
    exit(1);
  }

  // Set up modules for each motor
  for(size_t i = 0; i<nMotors; ++i){


    bool useDummyMotors = false;
    if(ctk::DMapFilesParser(".").getdMapFileElem(motorDriverCardDeviceNames[i]).uri == "/dummy/MotorDriverCard"){
      useDummyMotors = true;
      std::cout << "*** Setting up MotorDriverCard instance " << motorDriverCardDeviceNames[i] << " as a dummy." << std::endl;
    }
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(useDummyMotors);

    // Create a motor driver
    MotorDriverParameters dp{motorDriverCardDeviceNames[i], motorDriverCardModuleNames[i], motorDriverCardIds[i], motorDriverCardConfigFiles[i]};
    motorDriver.emplace_back(this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), dp);
    std::cout << "*** Created motor driver " << motorDriverCardIds[i] << " of card " << motorDriverCardModuleNames[i]
              << " on device " << motorDriverCardDeviceNames[i] << std::endl;

    motorDriver[i].controlInput.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["controlInput"]);
    motorDriver[i].hwReadback.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["hwReadback"]);
    motorDriver[i].swReadback.findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["swReadback"]);
//    motorDriver[i].findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]);


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

