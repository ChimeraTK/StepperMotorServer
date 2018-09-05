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
#include "ExecutionTimer.h"

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
struct MotorDriver : ctk::ModuleGroup {

  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
              const MotorDriverParameters &motorDriverParams,
              std::shared_ptr<ctk::StepperMotorUnitsConverter> unitsConverter);

  // Todo: We assume that the library is thread-safe. To be checked.
  std::shared_ptr<ctk::StepperMotor> _motor;
  std::shared_ptr<ctk::StepperMotorUnitsConverter> _motorUnitsConverter;


  /**
   * @class ControlInput
   * @details This module manages commands towards to motor driver.
   */
  struct ControlInput : ctk::ApplicationModule {

    ControlInput(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    funcmapT funcMap;
    std::shared_ptr<ctk::StepperMotor> _motor;
    ctk::ReadAnyGroup inputGroup;

    //ctk::ScalarPollInput<int32_t> systemIdle{this, "systemIdle", "", "System idle flag"};

//    TODO Equivalent reset method?
    ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CS"}};
    ctk::ScalarPushInput<int32_t> stopMotor{this, "stopMotor", "", "Stop the motor", {"CS"}};
    ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStopMotor", "", "Emergency stop motor", {"CS"}};
    ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]", {"CS"}};

    //FIXME Move dummy to this module
    ctk::ScalarOutput<int32_t> dummyMotorTrigger{this, "dummyMotorTrigger", "", "Triggers the dummy motor module after writing to a control input"};
    ctk::ScalarOutput<int32_t> dummyMotorStop{this, "dummyMotorStop","", "Stops the dummy motor"};

    //ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> startMotorRelative{this, "MOTOR_START_REL", "", "Start relative movement of motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CTRL"}};

//    ctk::ScalarPushInput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint", {"CS"}};
//    ctk::ScalarPushInput<double> relativePositionSetpoint{this, "relativePositionSetpoint", "", "Relative motor position setpoint", {"CS"}};
//    ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]", {"CS"}};
//    ctk::ScalarPushInput<int32_t> relativePositionSetpointInSteps{this, "relativePositionSetpointInSteps", "", "Relative motor position setpoint [steps]", {"CS"}};

    ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "enableSWPositionLimits", "", "Enable SW limits", {"CS"}};
    ctk::ScalarPushInput<float>   maxSWPositionLimit{this, "maxSWPositionLimit", "", "Positive SW position limit", {"CS"}};
    ctk::ScalarPushInput<float>   minSWPositionLimit{this, "minSWPositionLimit", "", "Negative SW position limit", {"CS"}};
    ctk::ScalarPushInput<int32_t> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "", "Positive SW position limit", {"CS"}};
    ctk::ScalarPushInput<int32_t> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "", "Negative SW position limit", {"CS"}};

    void prepare() override;
    void mainLoop() override;

  } controlInput{_motor, this, "controlInput", "Control inputs to the stepper motor"};



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

};

#endif /* INCLUDE_MOTORDRIVER_H_ */
