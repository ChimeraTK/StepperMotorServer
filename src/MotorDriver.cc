/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>
#include <mutex>


// Definitions of MotorUnitsConverter class
MotorUnitConverter::MotorUnitConverter(const float userUnitToStepsRatio, const std::string &userUnit)
  : _userUnitToStepsRatio(userUnitToStepsRatio), _userUnit(userUnit){}

float MotorUnitConverter::stepsToUnits(int steps){
  return _userUnitToStepsRatio * steps;
}

int MotorUnitConverter::unitsToSteps(float units){
  return units/_userUnitToStepsRatio;
}

// TODO Is there a better way to pass Module and Motor parameters?
MotorDriver::MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                         const MotorDriverParameters &driverParam,
                         bool isExtendedDriver,
                         std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter
                         )
  : ctk::ModuleGroup(owner, name, description, true),
    _motorUnitsConverter(unitsConverter)/*,
    _motor{new ctk::StepperMotor{driverParam.motorDriverCardDeviceName,
                                 driverParam.moduleName,
                                 driverParam.motorDriverId,
                                 driverParam.motorDriverCardConfigFileName,
                                 _motorUnitsConverter}},*/    {}

