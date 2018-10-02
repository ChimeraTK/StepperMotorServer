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


/**
 * @class MotorUnitsConverter
 * @brief This class provides a basic converter to relate the motor steps to a user-defined unit.
 */
class MotorUnitConverter : public ctk::StepperMotorUnitsConverter{
public:

  MotorUnitConverter(const float userUnitToStepsRatio, const std::string &userUnit);
  virtual ~MotorUnitConverter(){};

  float stepsToUnits(int steps);
  int unitsToSteps(float units);

  double geUserUnitToStepsRatio(){
    return _userUnitToStepsRatio;
  };

  std::string getUserUnit(){
    return _userUnit;
  }

private:
  const float _userUnitToStepsRatio;
  const std::string _userUnit;
};


/**
 * @class MotorDriver
 * @details Module group for all modules accessing the motor driver library.
 *          It provides shared objects provided by the library
 */
struct MotorDriver : public ctk::ModuleGroup {

  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
              std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter);

  virtual ~MotorDriver(){};

  // Todo: We assume that the library is thread-safe. To be checked. Use mutex
  std::shared_ptr<ctk::StepperMotorUnitsConverter> _motorUnitsConverter;
};


struct BasicMotorDriver : MotorDriver {
  BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter);

  std::shared_ptr<ctk::StepperMotor> motor;
  BasicControlInputHandler ctrlInputHandler;
  BasicHWReadbackHandler hwReadbackHandler;
  BasicSWReadbackHandler swReadbackHandler;
};


struct LinearMotorDriver : MotorDriver {

  LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter);

  std::shared_ptr<ctk::StepperMotorWithReference> motor;
  LinearMotorControlInputHandler ctrlInputHandler;
  ExtHWReadbackHandler hwReadbackHandler;
  ExtSWReadbackHandler swReadbackHandler;
};

#endif /* INCLUDE_MOTORDRIVER_H_ */
