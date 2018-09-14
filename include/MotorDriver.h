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
#include "ExecutionTimer.h"

#include "ControlInput.h"

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
  std::shared_ptr<ctk::StepperMotor> _motor;
  std::shared_ptr<ctk::StepperMotorUnitsConverter> _motorUnitsConverter;


//  BasicControlInput controlInput{_motor, this, "controlInput", "Control inputs to the stepper motor"};
   std::shared_ptr<ControlInputHandler> ctrlInputHandler;

  /**
   * @class HWReadback
   * @details This module cyclically reads data from the motor driver card.
   * TODO Which values must be passed through MotorControl and what can be piped to the CS directly?
   */
  struct HWReadback : ctk::ApplicationModule {

    HWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    std::shared_ptr<ctk::StepperMotor> _motor;
    ExecutionTimer<> execTimer;

    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

    ctk::ScalarOutput<int> isCalibrated{this, "isCalibrated", "", "Flag set to true if the motor is calibrated",{"CS"}};
    ctk::ScalarOutput<int32_t> motorErrorId{this, "motorError", "", "Error ID of the motor driver", {"CS"}};
    ctk::ScalarOutput<int> actualPositionInSteps{this, "actualPositionInSteps", "", "Actual position [steps]", {"CS"}};
    ctk::ScalarOutput<unsigned> decoderPosition{this, "decoderPosition", "nm", "Position measurement of the decoder", {"CS"}};

    // FIXME Debug RBVs
    ctk::ScalarOutput<int> enabledRBV{this, "enabledRBV", "", "Readback value of motor current enable", {"CS"}};
    ctk::ScalarOutput<int> targetPositionInStepsRBV{this, "targetPositionInStepsRBV", "", "Readback value of the target position", {"CS"}};

    // PVs having a static relation to HW readback values TODO Move to own module?
    ctk::ScalarOutput<float> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"CS"}};

    ctk::ScalarOutput<float> actualCycleTime{this, "actualCycleTime", "", "Actual cycle time by which the HW is being read", {"CS"}};

    void mainLoop() override;
  } hwReadback{_motor, this, "hwReadback", "Signals read from the motor driver HW"};


  /**
   * @class HWReadback
   * @details Variables read from the motor driver library (residing in SW)
   * TODO Which values must be passed through MotorControl and what can be piped to the CS directly?
   */
  struct SWReadBack : ctk::ApplicationModule {

    SWReadBack(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    std::shared_ptr<ctk::StepperMotor> _motor;

    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

    ctk::ScalarOutput<int> isSystemIdle{this, "isSystemIdle", "", "Flags if system is idle and a movement or calibration can be started", {"CS"}};
    ctk::ScalarOutput<std::string> motorState{this, "motorState", "", "State of the motor driver", {"CS"}};
    ctk::ScalarOutput<int> swPositionLimitsEnabled{this, "swPositionLimitsEnabled", "", "Flags if SW end switches are enabled.", {"CS"}};
    ctk::ScalarOutput<float> maxSWPositionLimit{this, "maxSWPositionLimit", "", "Currently set max. SW position limit", {"CS"}};
    ctk::ScalarOutput<float> minSWPositionLimit{this, "minSWPositionLimit", "", "Currently set min. SW position limit", {"CS"}};
    ctk::ScalarOutput<int> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "steps", "Currently set max. SW position limit", {"CS"}};
    ctk::ScalarOutput<int> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "steps", "Currently set min. SW position limit", {"CS"}};

    void mainLoop() override;
  } swReadback{_motor, this, "swReadback", "Signals read from the motor driver SW"};


}; /* MotorDriver */

struct BasicMotorDriver : MotorDriver {
  BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter)
    : MotorDriver(owner, name,description, driverParam, unitsConverter),
    _motor{std::make_shared<ctk::StepperMotor>(driverParam.motorDriverCardDeviceName,
        driverParam.moduleName,
        driverParam.motorDriverId,
        driverParam.motorDriverCardConfigFileName)} /*,
    ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", _motor} */ {
    ctrlInputHandler = std::make_shared<BasicControlInputHandler>(this, "controlInput", "Handles the control input to the motor driver.", _motor);
  };

  std::shared_ptr<ctk::StepperMotor> _motor;
  //BasicControlInputHandler ctrlInputHandler;

};


struct LinearMotorDriver : MotorDriver {

  LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const MotorDriverParameters &driverParam,
                    std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter)
    : MotorDriver(owner, name,description, driverParam, unitsConverter),
     _motor{std::make_shared<ctk::StepperMotorWithReference>(driverParam.motorDriverCardDeviceName,
                                                               driverParam.moduleName,
                                                               driverParam.motorDriverId,
                                                               driverParam.motorDriverCardConfigFileName)}/*,
     ctrlInputHandler{this, "controlInput", "Handles the control input to the motor driver.", _motor}*/ {

    ctrlInputHandler = std::make_shared<LinearMotorControlInputHandler>(this, "controlInput", "Handles the control input to the motor driver.", _motor);
  };

  std::shared_ptr<ctk::StepperMotorWithReference> _motor;
  //LinearMotorControlInputHandler ctrlInputHandler;

//    controlInput = std::make_shared<LinearMotorControlInput>(std::unique_ptr<BasicControlInput>(
//                                               new BasicControlInput(this, "controlInput", "Control inputs to the stepper motor")));

//  LinearMotorControlInput _linearMotorControlInput{new BasicControlInput{_motor, this, "controlInput", "Control inputs to the stepper motor"}};

};

#endif /* INCLUDE_MOTORDRIVER_H_ */
