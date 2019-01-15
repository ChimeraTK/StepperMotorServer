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
#include "mtca4u/MotorDriverCard/StepperMotor.h"
#include "ChimeraTK/DMapFilesParser.h"

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <algorithm>





static StepperMotorServer server;

static std::string basicLinearMotorType{"LinearMotor"};
static std::string linearMotorWithReferenceType{"LinearMotorWithReferenceSwitch"};

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

  //auto &cyclicTrigger = trigger.trigger;
  //ctk::VariableNetworkNode triggerC = cyclicTrigger;


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
  auto nMotors                    = config.get<uint32_t>("nMotors");
  auto motorType                  = config.get<std::vector<std::string>>("motorType");
  auto motorDriverCardDeviceNames = config.get<std::vector<std::string>>("motorDriverDeviceName");
  auto motorDriverCardModuleNames = config.get<std::vector<std::string>>("motorDriverModuleName");
  auto motorDriverCardIds         = config.get<std::vector<uint32_t>>("motorDriverId");
  auto motorDriverCardConfigFiles = config.get<std::vector<std::string>>("motorDriverConfigFile");
  auto encoderUnitToStepsRatios   = config.get<std::vector<double>>("encoderUnitToStepsRatio");

  auto userUnitToStepsRatios = config.get<std::vector<float>>("userUnitToStepsRatio");
  auto userPositionUnits     = config.get<std::vector<std::string>>("userPositionUnit");

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

    // Use a dummy motor, if requested
    bool useDummyMotors = false;
    if(ctk::DMapFilesParser(".").getdMapFileElem(motorDriverCardDeviceNames[i]).uri == "/dummy/MotorDriverCard"){
      useDummyMotors = true;
      std::cout << "*** Setting up MotorDriverCard instance " << motorDriverCardDeviceNames[i] << " as a dummy." << std::endl;
    }
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(useDummyMotors);


    // Motor configuration
    ctk::StepperMotorParameters motorParameters;
    motorParameters.deviceName     = motorDriverCardDeviceNames[i];
    motorParameters.moduleName     = motorDriverCardModuleNames[i];
    motorParameters.driverId       = motorDriverCardIds[i];
    motorParameters.configFileName = motorDriverCardConfigFiles[i];
    motorParameters.motorUnitsConverter
        = std::make_shared<ctk::StepperMotorUtility::StepperMotorUnitsScalingConverter>(userUnitToStepsRatios[i]);
    motorParameters.encoderUnitsConverter
        = std::make_shared<ctk::StepperMotorUtility::EncoderUnitsScalingConverter>(encoderUnitToStepsRatios[i]);


    // Configure motor driver HW
    if(initializedMotorDriverHW.count(motorDriverCardDeviceNames[i]) == 0 && !useDummyMotors){
      initMotorDriverHW(dMapFileName, motorDriverCardDeviceNames[i]);
    }

    // Create a motor driver according to the motor type
    if(config.get<std::vector<std::string>>("motorType")[i] == basicLinearMotorType){
      motorDriver.push_back(std::make_unique<BasicMotorDriver>(
          this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), motorParameters));
    }

    else if (config.get<std::vector<std::string>>("motorType")[i] == linearMotorWithReferenceType){
      motorDriver.push_back(std::make_unique<LinearMotorDriver>(
          this, "Motor"+std::to_string(i+1), "Driver of motor "+std::to_string(i+1), motorParameters));
    }
    else{
      std::cout << "!!! Terminating the server!" << std::endl
                << "Unknown motor type \"" << config.get<std::vector<std::string>>("motorType")[i]
                << " requested in MotorDriver configuration provided by" << serverConfigFile << "." << std::endl;
      exit(1);
    }

    std::cout << "*** Created motor driver " << motorDriverCardIds[i] << " of card " << motorDriverCardModuleNames[i]
              << " on device " << motorDriverCardDeviceNames[i] << ". Configuration file: " << motorDriverCardConfigFiles[i]
              << std::endl;

    motorDriver[i]->findTag("CS").connectTo(cs["Motor"+std::to_string(i+1)]);
    motorDriver[i]->flatten().findTag("TRIGGER").connectTo(trigger);

    //motorDriver[i]->operator []("hwReadback")("actualCycleTime") >> cs["Motor" + std::to_string(i+1)]("AnotherEnable");

    if(useDummyMotors){
      motorDummy.emplace_back(this, "MotorDummy"+std::to_string(i), "Dummy for motor"+std::to_string(i), motorParameters);
      motorDriver[i]->flatten().findTag("DUMMY").connectTo(motorDummy[i]);
    }
  }

  // Document module structure and connections
  motorDriver[0]->dumpGraph("motorDriverModuleGraph0.dot");
  motorDriver[1]->dumpGraph("motorDriverModuleGraph1.dot");

  dumpConnectionGraph();

} /* defineConnections() */


