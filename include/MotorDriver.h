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



struct MotorDriver : ctk::ModuleGroup {

  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description, const MotorDriverParameters &motorDriverParams);

  std::shared_ptr<ctk::StepperMotor> _motor;


//  //                                      Owner, name, unit, descr, tag

//  ctk::ScalarPushInput<double> speedLimitSetpoint{this, "SPEED_LIM_SETP", "", "Speed limit setpoint", {"CTRL"}};
//  ctk::ScalarPushInput<double> actualSpeedLimit{this, "ACT_SPEED_LIM", "", "Actual speed limit", {"CTRL"}};
//
//  // Current status to control system
//  ctk::ScalarOutput<double> actualPosition{this, "ACT_POS", "", "Actual position [scaled]", {"CTRL"}};
//  ctk::ScalarOutput<double> actualPositionInSteps{this, "ACT_POS_IN_STEPS", "", "Actual position [steps]", {"CTRL"}};
//  ctk::ScalarOutput<double> positiveEndSwitchPosition{this, "POS_END_SWITCH_POSITION", "", "Positive end switch position", {"CTRL"}}; /*TODO POS/NEG relevant, sensible?*/
//  ctk::ScalarOutput<double> negativeEndSwitchPosition{this, "NEG_END_SWITCH_POSITION", "", "Positive end switch position", {"CTRL"}}; /*TODO POS/NEG relevant, sensible?*/
//
//  ctk::ScalarOutput<int32_t> motorEnabled{this, "MOTOR_ENABLED", "", "Motor enabled flag", {"CTRL"}};
//  ctk::ScalarOutput<int32_t> motorStateOut{this, "MOTOR_STATE", "", "Motor state", {"CTRL"}};   // FIXME How to match this with the enum class state?
//  ctk::ScalarOutput<int32_t> motorStatus{this, "MOTOR_STATUS", "", "Motor status word", {"CTRL"}};  /*TODO Obsolete in high level of DriverLib?*/
//  ctk::ScalarOutput<int32_t> motorError{this, "MOTOR_ERROR", "", "Motor error word", {"CTRL"}};

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



  // TODO Which values must be passed through MotorControl and what can be piped to the CS directly?
  /** Values read from the motor driver hardware. */
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


  /**  Variables read from the motor driver library (residing in SW) */
  struct SWReadBack : ctk::ApplicationModule {

    SWReadBack(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    std::shared_ptr<ctk::StepperMotor> _motor;

    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

    ctk::ScalarOutput<int> isSystemIdle{this, "isSystemIdle", "", "Flags if system is idle and a movement or calibration can be started", {"CS"}};
    ctk::ScalarOutput<std::string> motorState{this, "motorState", "", "State of the motor driver", {"CS"}};
    ctk::ScalarOutput<int32_t> swPositionLimitsEnabled{this, "swPositionLimitsEnabled", "", "Flags if SW end switches are enabled.", {"CS"}};

    void mainLoop() override;
  } swReadback{_motor, this, "swReadback", "Signals read from the motor driver SW"};

};

#endif /* INCLUDE_MOTORDRIVER_H_ */
