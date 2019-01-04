/*
 * MotorDriver.h
 *
 *  Created on: Jul 16, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORDRIVER_H_
#define INCLUDE_MOTORDRIVER_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <mtca4u/MotorDriverCard/StepperMotor.h>
#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>

#include "ControlInput.h"
#include "MotorDriverReadback.h"

//FIXME Include the dummy in this module or reimplement the library's dummy, so we dont need this
#include "MotorDriverParameters.h"

#include <map>
#include <functional>
#include <memory>

namespace ctk = ChimeraTK;


typedef std::map<ctk::TransferElementID, std::function<void(void)>> funcmapT;


///**
// * @class MotorUnitsConverter
// * @brief This class provides a basic converter to relate the motor steps to a user-defined unit.
// */
//class MotorUnitConverter : public ctk::StepperMotorUnitsConverter{
//public:
//
//  MotorUnitConverter(const float userUnitToStepsRatio, const std::string &userUnit);
//  virtual ~MotorUnitConverter(){};
//
//  float stepsToUnits(int steps);
//  int unitsToSteps(float units);
//
//  double geUserUnitToStepsRatio(){
//    return _userUnitToStepsRatio;
//  };
//
//  std::string getUserUnit(){
//    return _userUnit;
//  }
//
//private:
//  const float _userUnitToStepsRatio;
//  const std::string _userUnit;
//};

struct BasicMotorDriver : public ctk::ModuleGroup {
  BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::unique_ptr<ctk::StepperMotorUtility::StepperMotorUnitsConverter> unitsConverter,
                    std::unique_ptr<ctk::StepperMotorUtility::EncoderUnitsConverter> encoderUnitsConverter);

  std::shared_ptr<ctk::StepperMotor> motor;
  BasicControlInputHandler ctrlInputHandler;
  BasicHWReadbackHandler hwReadbackHandler;
  //BasicSWReadbackHandler swReadbackHandler;
};


struct LinearMotorDriver : public ctk::ModuleGroup {

  LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::unique_ptr<ctk::StepperMotorUtility::StepperMotorUnitsConverter> unitsConverter,
                    std::unique_ptr<ctk::StepperMotorUtility::EncoderUnitsConverter> encoderUnitsConverter);

  std::shared_ptr<ctk::StepperMotorWithReference> motor;
  LinearMotorControlInputHandler ctrlInputHandler;
  ExtHWReadbackHandler hwReadbackHandler;
  //ExtSWReadbackHandler swReadbackHandler;
};

#endif /* INCLUDE_MOTORDRIVER_H_ */
