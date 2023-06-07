// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "StepperMotorServer.h"

#include "ChimeraTK/MotorDriverCard/StepperMotor.h"
#include "mtca4u/MotorDriverCard/MotorDriverCardFactory.h"
#include "version.h"
#include <unordered_set>

#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>
#include <ChimeraTK/DMapFileParser.h>

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ctkmot = ctk::MotorDriver;

// Definition of supported motor types
static constexpr char basicLinearMotorType[] = "Basic";
static constexpr char linearMotorWithReferenceType[] = "LinearMotorWithReferenceSwitch";

/**
 *  Hardware initialization
 *  This is called from within defineConnections() because there we have the device name available
 *  and this has to be done per FMC carrier board, not per motor (represented by the ApplicationModules).
 *  TODO Review this once additional driver HW gets supported
 */
static void initMotorDriverHW(const std::string& dMapFileName, const std::string& deviceAlias) {
  std::string command{"./initMotorDriverHW.py " + dMapFileName + " " + deviceAlias};

  int result = std::system(command.c_str());
  if(result != 0) {
    std::cout << "*** Error calling initialization script " << command << std::endl;
    std::exit(result);
  }
}

/********************************************************************************************************************/

[[noreturn]] static void terminateServer(const std::string& msg) {
  std::cout << std::endl << "!!! Terminating the server!" << std::endl << msg << std::endl;
  exit(1);
}

/********************************************************************************************************************/

StepperMotorServer::StepperMotorServer() : Application("steppermotorserver") {
  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor Server version " << AppVersion::major << "." << AppVersion::minor << "."
            << AppVersion::patch << std::endl;

  // Get and validate motor configuration
  auto nMotors = config.get<uint32_t>("Motors/nMotors");
  auto motorType = config.get<std::vector<std::string>>("Motors/motorType");
  auto motorDriverCardDeviceNames = config.get<std::vector<std::string>>("Motors/motorDriverDeviceName");
  auto motorDriverCardModuleNames = config.get<std::vector<std::string>>("Motors/motorDriverModuleName");
  auto motorDriverCardIds = config.get<std::vector<uint32_t>>("Motors/motorDriverId");
  auto motorDriverCardConfigFiles = config.get<std::vector<std::string>>("Motors/motorDriverConfigFile");
  auto encoderUnitToStepsRatios = config.get<std::vector<double>>("Motors/encoderUnitToStepsRatio");

  auto userUnitToStepsRatios = config.get<std::vector<float>>("Motors/userUnitToStepsRatio");
  auto userPositionUnits = config.get<std::vector<std::string>>("Motors/userPositionUnit");
  auto useDummyMotor = config.get<std::vector<int>>("Motors/dummy");

  if(motorDriverCardDeviceNames.size() != nMotors || motorDriverCardModuleNames.size() != nMotors ||
      motorDriverCardIds.size() != nMotors || motorDriverCardConfigFiles.size() != nMotors) {
    std::stringstream msg;
    msg << "A mismatch of dimensions in the MotorDriver configuration provided by ";
    msg << SERVER_CONFIG_FILE;
    terminateServer(msg.str());
  }

  // Set up modules for each motor
  std::unordered_set<std::string> initializedMotorDriverHW;
  for(size_t i = 0; i < nMotors; ++i) {
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(useDummyMotor[i] == 1);

    // Motor configuration
    ctkmot::StepperMotorParameters motorParameters;
    motorParameters.deviceName = motorDriverCardDeviceNames[i];
    motorParameters.moduleName = motorDriverCardModuleNames[i];
    motorParameters.driverId = motorDriverCardIds[i];
    motorParameters.configFileName = motorDriverCardConfigFiles[i];
    motorParameters.motorUnitsConverter =
        std::make_shared<ctkmot::utility::ScalingMotorStepsConverter>(userUnitToStepsRatios[i]);
    motorParameters.encoderUnitsConverter =
        std::make_shared<ctkmot::utility::ScalingEncoderStepsConverter>(encoderUnitToStepsRatios[i]);

    // Configure motor driver HW
    if(initializedMotorDriverHW.count(motorDriverCardDeviceNames[i]) == 0 && (useDummyMotor[i] == 0)) {
      initMotorDriverHW(std::string{DMAP_FILE_NAME}, motorDriverCardDeviceNames[i]);
    }

    // Create a motor driver according to the motor type
    if(motorType[i] == basicLinearMotorType) {
      motorParameters.motorType = ctkmot::StepperMotorType::BASIC;
    }
    else if(motorType[i] == linearMotorWithReferenceType) {
      motorParameters.motorType = ctkmot::StepperMotorType::LINEAR;
    }
    else {
      std::stringstream msg;
      msg << "Unknown motor type \"" << motorType[i]
          << "\" requested in MotorDriver configuration provided by: " << std::endl
          << SERVER_CONFIG_FILE << "." << std::endl;

      terminateServer(msg.str());
    }

    motorDriver.emplace_back(this, "Motor" + std::to_string(i + 1), "Driver of motor " + std::to_string(i + 1),
        motorParameters, "/Trigger/voidTick");

    std::cout << "*** Created motor driver " << motorDriverCardIds[i] << " of card " << motorDriverCardModuleNames[i]
              << " on device " << motorDriverCardDeviceNames[i]
              << ". Configuration file: " << motorDriverCardConfigFiles[i]
              << " Motor is dummy: " << std::to_string(useDummyMotor[i] == 1) << std::endl;

    if(useDummyMotor[i] == 1) {
      motorDummy.emplace_back(
          this, "Motor" + std::to_string(i + 1), "Dummy for motor " + std::to_string(i), motorParameters);
    }
  }
}
