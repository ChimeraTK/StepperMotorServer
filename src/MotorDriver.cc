/*
 * MotorDriver.cc
 *
 *  Created on: Jul 17, 2018
 *      Author: ckampm
 */

#include "MotorDriver.h"
#include <iostream>
#include <mutex>
#include <memory>


//// Definitions of MotorUnitsConverter class
//MotorUnitConverter::MotorUnitConverter(const float userUnitToStepsRatio, const std::string &userUnit)
//  : _userUnitToStepsRatio(userUnitToStepsRatio), _userUnit(userUnit){}
//
//float MotorUnitConverter::stepsToUnits(int steps){
//  return _userUnitToStepsRatio * steps;
//}
//
//int MotorUnitConverter::unitsToSteps(float units){
//  return units/_userUnitToStepsRatio;
//}


BasicMotorDriver::BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                   const MotorDriverParameters &driverParam,
                                   std::unique_ptr<ctk::StepperMotorUtility::StepperMotorUnitsConverter> unitsConverter,
                                   std::unique_ptr<ctk::StepperMotorUtility::EncoderUnitsConverter> encoderUnitsConverter)
  : ctk::ModuleGroup(owner, name, description),
  motor{std::make_shared<ctk::StepperMotor>(driverParam.motorDriverCardDeviceName,
                              driverParam.moduleName,
                              driverParam.motorDriverId,
                              driverParam.motorDriverCardConfigFileName,
                              std::move(unitsConverter),
                              std::move(encoderUnitsConverter))},
  ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", motor},
  hwReadbackHandler{motor, this, "hwReadback", "Signals read from the motor driver HW"}{}//,
  //swReadbackHandler{motor, this, "swReadback", "Signals read from the motor driver SW"} {}


LinearMotorDriver::LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                     const MotorDriverParameters &driverParam,
                                     std::unique_ptr<ctk::StepperMotorUtility::StepperMotorUnitsConverter> unitsConverter,
                                     std::unique_ptr<ctk::StepperMotorUtility::EncoderUnitsConverter> encoderUnitsConverter)
  : ctk::ModuleGroup(owner, name, description),
    motor{std::make_shared<ctk::StepperMotorWithReference>(driverParam.motorDriverCardDeviceName,
                                                           driverParam.moduleName,
                                                           driverParam.motorDriverId,
                                                           driverParam.motorDriverCardConfigFileName,
                                                           std::move(unitsConverter),
                                                           std::move(encoderUnitsConverter))},
    ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", motor},
    hwReadbackHandler{motor, this, "hwReadback", "Signals read from the motor driver HW"}{}//,
    //swReadbackHandler{motor, this, "swReadback", "Signals read from the motor driver SW"} {}
