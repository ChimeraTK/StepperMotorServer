/*
 * MotorDriverReadBack.h
 *
 *  Created on: Sep 17, 2018
 *      Author: ckampm
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

struct ReadbackValues : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;
  //virtual void readback() = 0;
};

struct HWReadbackValues : ReadbackValues {

  HWReadbackValues(/*std::shared_ptr<ctk::StepperMotor> motor, */ctk::EntityOwner *owner, const std::string &name, const std::string &description)
    : ReadbackValues(owner, name, description, true)/*, _motor(motor)*/ {};

  //std::shared_ptr<ctk::StepperMotor> _motor;

  ctk::ScalarOutput<int> isCalibrated{this, "isCalibrated", "", "Flag set to true if the motor is calibrated",{"CS"}};
  ctk::ScalarOutput<int32_t> motorErrorId{this, "motorError", "", "Error ID of the motor driver", {"CS"}};
  ctk::ScalarOutput<int> actualPositionInSteps{this, "actualPositionInSteps", "", "Actual position [steps]", {"CS"}};
  ctk::ScalarOutput<unsigned> decoderPosition{this, "decoderPosition", "nm", "Position measurement of the decoder", {"CS"}};

  // FIXME Debug RBVs
  ctk::ScalarOutput<int> enabledRBV{this, "enabledRBV", "", "Readback value of motor current enable", {"CS"}};
  ctk::ScalarOutput<int> targetPositionInStepsRBV{this, "targetPositionInStepsRBV", "", "Readback value of the target position", {"CS"}};

  // PVs having a static relation to HW readback values TODO Move to own module?
  ctk::ScalarOutput<float> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"CS"}};

  //virtual void readback() override;
};

struct HWReadbackValuesExt : ReadbackValues {

  HWReadbackValuesExt(/*std::shared_ptr<ctk::StepperMotor> motor, */ctk::EntityOwner *owner, const std::string &name, const std::string &description)
    :ReadbackValues(/*motor, */owner, name, description, true)/*, _motor(std::dynamic_pointer_cast<ctk::StepperMotorWithReference>(motor))*/ {};

  //std::shared_ptr<ctk::StepperMotorWithReference> _motor;

  ctk::ScalarOutput<int> isPositiveReferenceActive{this, "isPositiveReferenceActive", "", "Status of positive end switch",{"CS"}};
  ctk::ScalarOutput<int> isNegativeReferenceActive{this, "isNegativeReferenceActive", "", "Status of negative end switch",{"CS"}};

  //virtual void readback() override;
};

/**
 * @class ReadbackHandler
 * @brief This module cyclically reads data from the motor driver card.
 */
class ReadbackHandler : public ctk::ApplicationModule {

public:
  ReadbackHandler(/*std::shared_ptr<ctk::StepperMotor> motor,*/
                  ctk::EntityOwner *owner, const std::string &name, const std::string &description);
  using ctk::ApplicationModule::ApplicationModule;

  ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

  // Diagnostics
  ctk::ScalarOutput<float> actualCycleTime{this, "actualCycleTime", "", "Actual cycle time by which the HW is being read", {"CS"}};
  ctk::ScalarOutput<float> actualReceiveTime{this, "actualReceiveTime", "", "Actual time required to read all variables in this module from the HW.", {"CS"}};

  virtual void mainLoop() override;

protected:
  std::function<void(void)> readbackFunction;

private:
//  std::shared_ptr<ctk::StepperMotor> _motor;
//  std::unique_ptr<ReadbackValues> _readbackValues;
  ExecutionTimer<> execTimer;
  ExecutionTimer<> receiveTimer;
};

class BasicHWReadbackHandler : public ReadbackHandler {

public:

  BasicHWReadbackHandler(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotor> _motor;
  HWReadbackValues hwReadbackValues;

  virtual void prepare();
  //virtual void mainLoop();

protected:
  void readback();
};


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
 * @class HWReadback
 * @brief Variables read from the motor driver library (residing in SW)
 */
struct SWReadback : ctk::ApplicationModule {

  SWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotor> _motor;
  ExecutionTimer<> receiveTimer;

  ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

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

  //Diagnostics
  ctk::ScalarOutput<float> actualReceiveTime{this, "actualReceiveTime", "", "Actual time required to get all variables in this module from the driver instance.", {"CS"}};

  void mainLoop() override;
};



#endif /* INCLUDE_MOTORDRIVERREADBACK_H_ */
