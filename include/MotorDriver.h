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
 * @details This class provides a basic converter to relate the motor steps to a user-defined unit.
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
              const MotorDriverParameters &motorDriverParams,
              std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter);

  virtual ~MotorDriver(){};

  // Todo: We assume that the library is thread-safe. To be checked.
  //std::shared_ptr<ctk::StepperMotor> _motor;
  std::shared_ptr<ctk::StepperMotorUnitsConverter> _motorUnitsConverter;


  std::shared_ptr<ControlInputHandler> ctrlInputHandler;
  std::unique_ptr<ReadbackHandler> hwReadbackHandler;
  std::unique_ptr<ReadbackHandler> swReadbackHandler;

}; /* MotorDriver */

struct BasicMotorDriver : MotorDriver {
  BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter)
    : MotorDriver(owner, name,description, driverParam, unitsConverter),
    _motor{std::make_shared<ctk::StepperMotor>(driverParam.motorDriverCardDeviceName,
        driverParam.moduleName,
        driverParam.motorDriverId,
        driverParam.motorDriverCardConfigFileName,
        unitsConverter)}/*,
    ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", _motor} */
  {

//    ctrlInputHandler = std::make_shared<ControlInputHandler>(this, "controlInput", "Handles the control input to the motor driver.", _motor);
    ctrlInputHandler = std::make_shared<BasicControlInputHandler>(this, "controlInput", "Handles the control input to the motor driver.", _motor);
    hwReadbackHandler = std::move(std::unique_ptr<BasicHWReadbackHandler>{new BasicHWReadbackHandler(_motor, this, "hwReadback", "Signals read from the motor driver HW")});
    swReadbackHandler = std::move(std::unique_ptr<BasicSWReadbackHandler>{new BasicSWReadbackHandler(_motor, this, "swReadback", "Signals read from the motor driver SW")});
  };

  std::shared_ptr<ctk::StepperMotor> _motor;
  //BasicHWReadbackHandler hwReadbackHandler;
  //BasicControlInputHandler ctrlInputHandler;

}; /* BasicMotorDriver */


struct LinearMotorDriver : MotorDriver {

  LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter)
    : MotorDriver(owner, name,description, driverParam, unitsConverter),
     _motor{std::make_shared<ctk::StepperMotorWithReference>(driverParam.motorDriverCardDeviceName,
                                                               driverParam.moduleName,
                                                               driverParam.motorDriverId,
                                                               driverParam.motorDriverCardConfigFileName,
                                                               unitsConverter)}
    /*,
    ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", _motor}*/ {
    ctrlInputHandler = std::make_shared<LinearMotorControlInputHandler>(this, "controlInput", "Handles the control input to the motor driver.", _motor);
    hwReadbackHandler = std::move(std::unique_ptr<ExtHWReadbackHandler>{new ExtHWReadbackHandler(_motor, this, "hwReadback", "Signals read from the motor driver HW")});
    swReadbackHandler = std::move(std::unique_ptr<ExtSWReadbackHandler>{new ExtSWReadbackHandler(_motor, this, "swReadback", "Signals read from the motor driver SW")});
  };

  std::shared_ptr<ctk::StepperMotorWithReference> _motor;

};

#endif /* INCLUDE_MOTORDRIVER_H_ */
