/*
 * MotorCtrl.h
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORCONTROL_H_
#define INCLUDE_MOTORCONTROL_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>

namespace ctk = ChimeraTK;

struct MotorControl : ctk::ApplicationModule{

  using ctk::ApplicationModule::ApplicationModule;

  enum class MotorState : int32_t {
    MOTOR_READY=0, MOTOR_ERROR=1, MOTOR_RUNNING=2, MOTOR_RESETTING=3, MOTOR_DISABLED=4
  } motorState;

  //                                      Owner, name, unit, descr, tags
  ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> startMotorRelative{this, "MOTOR_START_REL", "", "Start relative movement of motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> stopMotor{this, "stop", "", "Stop the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStop", "", "Emergency stop motor", {"CS"}};

  ctk::ScalarPushInput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint [scaled]", {"CS"}};
  ctk::ScalarPushInput<double> relativeMotorPositionSetpoint{this, "REL_MOT_POS_SETP", "", "Relative motor position setpoint [scaled]", {"CS"}};
  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "MOT_POS_SETP_IN_STEPS", "", "Motor position setpoint [steps]", {"CS"}};
  ctk::ScalarPushInput<int32_t> relativeMotorPositionSetpointInSteps{this, "REL_MOT_POS_SETP_IN_STEPS", "", "Relative motor position setpoint [steps]", {"CS"}};

  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "SW_LIM_ENABLE", "", "Enable SW limits", {"CS"}};
  ctk::ScalarPushInput<double> positiveSWPositionLimit{this, "POS_SW_POS_LIM", "", "Positive SW position limit", {"CS"}};
  ctk::ScalarPushInput<double> negativeSWPositionLimit{this, "NEG_SW_POS_LIM", "", "Negative SW position limit", {"CS"}};

  ctk::ScalarPushInput<double> speedLimitSetpoint{this, "SPEED_LIM_SETP", "", "Speed limit setpoint", {"CS"}};
  ctk::ScalarPushInput<double> actualSpeedLimit{this, "ACT_SPEED_LIM", "", "Actual speed limit", {"CS"}};

  // Current status to control system
  ctk::ScalarOutput<double> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"CS"}};
  ctk::ScalarOutput<double> actualPositionInSteps{this, "ACT_POS_IN_STEPS", "", "Actual position [steps]", {"CS"}};
  ctk::ScalarOutput<double> positiveEndSwitchPosition{this, "POS_END_SWITCH_POSITION", "", "Positive end switch position", {"CS"}}; /*TODO POS/NEG relevant, sensible?*/
  ctk::ScalarOutput<double> negativeEndSwitchPosition{this, "NEG_END_SWITCH_POSITION", "", "Positive end switch position", {"CS"}}; /*TODO POS/NEG relevant, sensible?*/

  ctk::ScalarOutput<int32_t> motorEnabled{this, "MOTOR_ENABLED", "", "Motor enabled flag", {"CS"}};
  ctk::ScalarOutput<int32_t> motorStateOut{this, "MOTOR_STATE", "", "Motor state", {"CS"}};   // FIXME How to match this with the enum class state?
  ctk::ScalarOutput<int32_t> motorStatus{this, "MOTOR_STATUS", "", "Motor status word", {"CS"}};  /*TODO Obsolete in high level of DriverLib?*/
  ctk::ScalarOutput<int32_t> motorError{this, "MOTOR_ERROR", "", "Motor error word", {"CS"}};

  struct ToMotorDriver : ctk::VariableGroup {
    using ctk::VariableGroup::VariableGroup;

    ctk::ScalarOutput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint"};
    ctk::ScalarOutput<double> relativePositionSetpoint{this, "relativePositionSetpoint", "", "Relative motor position setpoint"};
    ctk::ScalarOutput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]"};
    ctk::ScalarOutput<int32_t> relativePositionSetpointInSteps{this, "relativePositionSetpointInSteps", "", "Relative motor position setpoint [steps]"};
  } toMotorDriver{this, "toMotorDriver", "Control inputs to the motor driver"};


  // FIXME For testing
  ctk::ScalarPollInput<double> currentPosition{this, "currentPosition", "", ""};

  ctk::ReadAnyGroup inputGroup;

  void mainLoop() override;

  // Motor states
  //FIXME Better be private
  void stateMotorDisabled();
  void stateMotorReady();
  void stateMotorRunning();
  void stateMotorError();
  void stateMotorResetting();

}; /* struct MotorControl */
#endif /* INCLUDE_MOTORCONTROL_H_ */
