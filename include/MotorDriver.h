/*
 * MotorDriver.h
 *
 *  Created on: Jul 16, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORDRIVER_H_
#define INCLUDE_MOTORDRIVER_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <mtca4u/MotorDriverCard/StepperMotor.h>

#include <map>
#include <functional>
#include <memory>

namespace ctk = ChimeraTK;


typedef std::map<ctk::TransferElementID, std::function<void(void)>> funcmapT;


struct MotorDriver : ctk::ModuleGroup {

  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotor> _motor;


//  //                                      Owner, name, unit, descr, tags
//  ctk::ScalarPushInput<int32_t> enableMotor{this, "MOTOR_ENABLE", "", "Enable the motor", {"CTRL"}};


//  ctk::ScalarPushInput<int32_t> stopMotor{this, "MOTOR_STOP", "", "Stop the motor", {"CTRL"}};
//  ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CTRL"}};
//  ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "MOTOR_EMERGENCY_STOP", "", "Emergency stop motor", {"CTRL"}};



//  ctk::ScalarPushInput<double> positionSetpoint{this, "MOT_POS_SETP", "", "Motor position setpoint [scaled]", {"CTRL"}};
//  ctk::ScalarPushInput<double> relativeMotorPositionSetpoint{this, "REL_MOT_POS_SETP", "", "Relative motor position setpoint [scaled]", {"CTRL"}};
//  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "MOT_POS_SETP_IN_STEPS", "", "Motor position setpoint [steps]", {"CTRL"}};
//  ctk::ScalarPushInput<int32_t> relativeMotorPositionSetpointInSteps{this, "REL_MOT_POS_SETP_IN_STEPS", "", "Relative motor position setpoint [steps]", {"CTRL"}};
//
//  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "SW_LIM_ENABLE", "", "Enable SW limits", {"CTRL"}};
//  ctk::ScalarPushInput<double> positiveSWPositionLimit{this, "POS_SW_POS_LIM", "", "Positive SW position limit", {"CTRL"}};
//  ctk::ScalarPushInput<double> negativeSWPositionLimit{this, "NEG_SW_POS_LIM", "", "Negative SW position limit", {"CTRL"}};
//
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

  struct ControlInputs : ctk::ApplicationModule {

    ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    funcmapT funcMap;
    std::shared_ptr<ctk::StepperMotor> _motor;

//    FIXME Equivalents for start and reset methods
    ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> startMotorRelative{this, "MOTOR_START_REL", "", "Start relative movement of motor", {"CTRL"}};
    ctk::ScalarPushInput<int32_t> stopMotor{this, "stop", "", "Stop the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CTRL"}};
    ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStop", "", "Emergency stop motor", {"CTRL"}};

    ctk::ScalarPushInput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint [scaled]", {"CTRL"}};

    //FIXME
    ctk::ScalarOutput<double> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"MEAS"}};

    void prepare() override;
    void mainLoop() override;
  } controlInputs{_motor, this, "ControlInputs", "Control inputs to the stepper motor"};
};



#endif /* INCLUDE_MOTORDRIVER_H_ */
