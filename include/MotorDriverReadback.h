/**
 * MotorDriverReadBack.h
 *
 *  Created on: Sep 17, 2018
 *      Author: ckampm
 *
 */

#ifndef INCLUDE_MOTORDRIVERREADBACK_H_
#define INCLUDE_MOTORDRIVERREADBACK_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <mtca4u/MotorDriverCard/StepperMotor.h>
#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>
#include "ExecutionTimer.h"

#include <functional>

namespace ctk = ChimeraTK;


/**
 *  @class HWReadbackValues
 *  @brief A variable group of basic values that are read from the motor driver HW.
 *
 *  These are the basic variables read from the motor driver HW for a simple motor. In addition to variables read from HW,
 *  it contains variables which have a static relation to those values, e.g. scaled position values.
 */
struct HWReadbackValues : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarOutput<int> isCalibrated{this, "isCalibrated", "", "Flag set to true if the motor is calibrated",{"CS"}};
  ctk::ScalarOutput<int32_t> motorErrorId{this, "motorError", "", "Error ID of the motor driver", {"CS"}};
  ctk::ScalarOutput<int> actualPositionInSteps{this, "actualPositionInSteps", "", "Actual position [steps]", {"CS"}};
  ctk::ScalarOutput<unsigned> decoderPosition{this, "decoderPosition", "nm", "Position measurement of the decoder", {"CS"}};

  // FIXME Debug RBVs
  ctk::ScalarOutput<int> enabledRBV{this, "enabledRBV", "", "Readback value of motor current enable", {"CS"}};
  ctk::ScalarOutput<int> targetPositionInStepsRBV{this, "targetPositionInStepsRBV", "", "Readback value of the target position", {"CS"}};

  // PVs having a static relation to HW readback values TODO Move to own module?
  ctk::ScalarOutput<float> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"CS"}};
};

/**
 *  @class HWReadbackValuesExt
 *  @brief A variable group related to reference switches, read from the motor driver HW.
 */
struct HWReadbackValuesExt : ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarOutput<int> isPositiveReferenceActive{this, "isPositiveReferenceActive", "", "Status of positive end switch",{"CS"}};
  ctk::ScalarOutput<int> isNegativeReferenceActive{this, "isNegativeReferenceActive", "", "Status of negative end switch",{"CS"}};
};

/**
 *  @class SWReadbackValues
 *  @brief A variable group of basic values that are read from the motor driver library (residing in SW).
 *
 *  These are the basic variables read from the motor driver HW for a simple motor.
 */
struct SWReadbackValues : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarOutput<int> isSystemIdle{this, "isSystemIdle", "", "Flags if system is idle and a movement or calibration can be started", {"CS"}};
  ctk::ScalarOutput<std::string> motorState{this, "motorState", "", "State of the motor driver", {"CS"}};
  ctk::ScalarOutput<int> swPositionLimitsEnabled{this, "swPositionLimitsEnabled", "", "Flags if SW end switches are enabled.", {"CS"}};
  ctk::ScalarOutput<float> maxSWPositionLimit{this, "maxSWPositionLimit", "", "Currently set max. SW position limit", {"CS"}};
  ctk::ScalarOutput<float> minSWPositionLimit{this, "minSWPositionLimit", "", "Currently set min. SW position limit", {"CS"}};
  ctk::ScalarOutput<int> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "steps", "Currently set max. SW position limit", {"CS"}};
  ctk::ScalarOutput<int> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "steps", "Currently set min. SW position limit", {"CS"}};

  ctk::ScalarOutput<double> currentLimit{this, "currentLimit", "A", "Current limit set for the motor", {"CS"}}; //TODO Implement
  ctk::ScalarOutput<double> maxCurrentLimit{this, "maxCurrentLimit", "A", "Current limit set for the motor", {"CS"}}; //TODO Implement
  ctk::ScalarOutput<double> speedLimit{this, "speedLimit", "", "Speed limit set for the motor", {"CS"}};
  ctk::ScalarOutput<double> maxSpeedCapability{this, "maxSpeedCapability", "", "Maximum velocity of the motor", {"CS"}};

  ctk::ScalarOutput<int32_t> isFullStepping{this, "isFullStepping", "", "Flags if full-stepping mode of the driver is active.", {"CS"}};
  ctk::ScalarOutput<int32_t> autostartEnabled{this, "autostartEnabled", "", "Flags if autostart mode is active", {"CS"}};
};


/**
 *  @class SWReadbackValuesExt
 *  @brief A variable group related to reference switches, read from the motor driver library (residing in SW).
 */
struct SWReadbackValuesExt : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarOutput<int>   positiveEndReferenceInSteps{this, "positiveEndReferenceInSteps", "steps", "Position of the positive reference switch",{"CS"}};
  ctk::ScalarOutput<int>   negativeEndReferenceInSteps{this, "negativeEndReferenceInSteps", "steps", "Position of the positive reference switch",{"CS"}};
  ctk::ScalarOutput<float> positiveEndReference{this, "positiveEndReference", "", "Position of the positive reference switch",{"CS"}};
  ctk::ScalarOutput<float> negativeEndReference{this, "negativeEndReference", "", "Position of the positive reference switch",{"CS"}};
  ctk::ScalarOutput<float> positiveEndReferenceTolerance{this, "positiveEndReferenceTolerance", "", "Tolerance of the calibrated positive end switch position.", {"CS"}};
  ctk::ScalarOutput<float> negativeEndReferenceTolerance{this, "negativeEndReferenceTolerance", "", "Tolerance of the calibrated negative end switch position.", {"CS"}};
};


/**
 *  @class ReadbackHandler
 *  @brief Base application module for cyclically reading data from the motor driver card HW.
 */
class ReadbackHandler : public ctk::ApplicationModule {

public:

  using ctk::ApplicationModule::ApplicationModule;

  ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW", {"TRIGGER"}};

  // Diagnostics
  ctk::ScalarOutput<float> actualCycleTime{this, "actualCycleTime", "ms", "Actual cycle time by which the HW is being read", {"CS"}};
  ctk::ScalarOutput<float> actualReceiveTime{this, "actualReceiveTime", "ms", "Actual time required to read all variables in this module from the HW.", {"CS"}};

  virtual void mainLoop() override;

protected:
  std::function<void(void)> readbackFunction;

private:
  ExecutionTimer<> execTimer;
  ExecutionTimer<> receiveTimer;
};

/**
 *  @class BasicHWReadbackHandler
 *  @brief An application module to cyclically read data from the motor driver HW for a simple motor without reference switches.
 *
 *  This module supports a simple motor. It provides to functionality of the ChimeraTK::StepperMotor class of the MotorDriverCard library.
 */
class BasicHWReadbackHandler : public ReadbackHandler {

public:

  BasicHWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotor> _motor;
  HWReadbackValues hwReadbackValues;

  virtual void prepare();

protected:
  void readback();
};


/**
 *  @class ExtHWReadbackHandler
 *  @brief An application module to cyclically read data from the motor driver HW for a motor with reference switches.
 *
 *  This module supports a linear motor with reference switches . It provides to functionality of the ChimeraTK::StepperMotorWithRefernce
 *  class of the MotorDriverCard library.
 */
class ExtHWReadbackHandler : public BasicHWReadbackHandler {
public:
  ExtHWReadbackHandler(std::shared_ptr<ctk::StepperMotorWithReference> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotorWithReference> _motor;
  HWReadbackValuesExt hwReadbackValues;

  virtual void prepare();

protected:
  void readback();
};


/**
 *  @class BasicSWReadbackHandler
 *  @brief An application module to cyclically read data (residing in SW) for a simple motor without reference switches.
 *
 *  This module supports a simple motor. It provides to functionality of the ChimeraTK::StepperMotor class of the MotorDriverCard library.
 */
class BasicSWReadbackHandler : public ReadbackHandler {
public:

  BasicSWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotor> _motor;
  SWReadbackValues swReadbackValues;

  virtual void prepare() override;

protected:
  void readback();
};


/**
 *  @class ExtSWReadbackHandler
 *  @brief An application module to cyclically read data (residing in SW) for a simple motor with reference switches.
 *
 *  This module supports a linear motor with reference switches . It provides to functionality of the ChimeraTK::StepperMotorWithRefernce
 *  class of the MotorDriverCard library
 */
class ExtSWReadbackHandler : public BasicSWReadbackHandler {
public:

  ExtSWReadbackHandler(std::shared_ptr<ctk::StepperMotorWithReference> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotorWithReference> _motor;
  SWReadbackValuesExt swReadbackValues;

  virtual void prepare() override;

protected:
  void readback();
};

#endif /* INCLUDE_MOTORDRIVERREADBACK_H_ */
