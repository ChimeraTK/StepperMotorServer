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
#include "ChimeraTK/DMapFilesParser.h"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <algorithm>



static StepperMotorServer server;

/** Hardware initialization
 *  This is called from within defineConnections() because there we have the device name available
 *  and this has to be done per FMC carrier board, not per motor (represented by the ApplicationModules).
 *  TODO Review this once additional driver HW gets supported
 */
static void initMotorDriverHW(std::string dMapFileName, std::string deviceAlias){
  std::string command{"./initMotorDriverHW.py " + dMapFileName + " " + deviceAlias};

  int result = std::system(command.c_str());
  if(result != 0) {
    std::cout << "*** Error calling initialization script " << command << std::endl;
    std::exit(result);
  }
};

/** Define interconnection of modules */
void StepperMotorServer::defineConnections(){

  std::string dMapFileName{"dummies.dmap"};

  ChimeraTK::setDMapFilePath(dMapFileName);

  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor Server version "
            << AppVersion::major << "." << AppVersion::minor << "." << AppVersion::patch << std::endl;


  // Setup poll trigger
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs["Timer"]("updateOnce") >> trigger.forceUpdate;
  config("cycleTimeEnable") >> trigger.automaticUpdate;
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

  std::vector<float> userUnitToStepsRatios       = config.get<std::vector<float>>("userUnitToStepsRatio");
  std::vector<std::string> userPositionUnits     = config.get<std::vector<std::string>>("userPositionUnit");

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
  std::unordered_set<std::string> initializedMotorDriverHW;
  for(size_t i = 0; i<nMotors; ++i){

    bool useDummyMotors = false;
    if(ctk::DMapFilesParser(".").getdMapFileElem(motorDriverCardDeviceNames[i]).uri == "/dummy/MotorDriverCard"){
      useDummyMotors = true;
      std::cout << "*** Setting up MotorDriverCard instance " << motorDriverCardDeviceNames[i] << " as a dummy." << std::endl;
    }
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(useDummyMotors);

    // Motor data
    MotorDriverParameters driverParams{motorDriverCardDeviceNames[i], motorDriverCardModuleNames[i], motorDriverCardIds[i], motorDriverCardConfigFiles[i]};
    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter = std::make_shared<MotorUnitConverter>(userUnitToStepsRatios[i], userPositionUnits[i]);

    // Configure motor driver HW
    if(initializedMotorDriverHW.count(motorDriverCardDeviceNames[i]) == 0 && !useDummyMotors){
      initMotorDriverHW(dMapFileName, motorDriverCardDeviceNames[i]);
    }

    // Create a motor driver according to the motor type
    if(i == 0){
      motorDriver.push_back(std::make_shared<BasicMotorDriver>(this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), driverParams, unitsConverter));
    }
    else{
      motorDriver.push_back(std::make_shared<LinearMotorDriver>(this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), driverParams, unitsConverter));
    }
    //motorDriver.emplace_back(this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), "BASIC_STEPPER_MOTOR",  driverParams, unitsConverter);

    std::cout << "*** Created motor driver " << motorDriverCardIds[i] << " of card " << motorDriverCardModuleNames[i]
              << " on device " << motorDriverCardDeviceNames[i] << ". Configuration file: " << motorDriverCardConfigFiles[i]
              <<std::endl;

    motorDriver[i]->ctrlInputHandler->findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["controlInput"]);
    motorDriver[i]->hwReadbackHandler->findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["hwReadback"]);
    motorDriver[i]->swReadback->findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]["swReadback"]);
//    motorDriver[i].findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]);

    cyclicTrigger >> (*(motorDriver[i]->hwReadbackHandler))("trigger");
    cyclicTrigger >> (*(motorDriver[i]->swReadback))("trigger");

    if(useDummyMotors){
      motorDummy.emplace_back(this, "MotorDummy"+std::to_string(i), "Dummy for motor"+std::to_string(i), driverParams);
      motorDriver[i]->ctrlInputHandler->findTag("DUMMY").connectTo(motorDummy[i]);
//      motorDriver[i]->ctrlInputHandler("dummyMotorTrigger") >> motorDummy[i]("trigger");
//      motorDriver[i]->ctrlInputHandler("dummyMotorStop") >> motorDummy[i]("stop");
    }
  }

  // Document module structure and connections
  motorDriver[0]->dumpGraph("motorDriverModuleGraph0.dot");
  motorDriver[1]->dumpGraph("motorDriverModuleGraph1.dot");

  dumpConnectionGraph();

} /* defineConnections() */


