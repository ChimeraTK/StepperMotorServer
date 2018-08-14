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

  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description, MotorDriverParameters motorDriverParams);

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

  struct ControlInputs : ctk::ApplicationModule {

    ControlInputs(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    funcmapT funcMap;
    std::shared_ptr<ctk::StepperMotor> _motor;
    ctk::ReadAnyGroup inputGroup;

    ctk::ScalarPollInput<int32_t> systemIdle{this, "systemIdle", "", "System idle flag"};

//    FIXME Equivalents for start and reset methods
    ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> startMotorRelative{this, "MOTOR_START_REL", "", "Start relative movement of motor", {"CTRL"}};
    ctk::ScalarPushInput<int32_t> stopMotor{this, "stop", "", "Stop the motor", {"CTRL"}};
    //ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CTRL"}};
    ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStop", "", "Emergency stop motor", {"CTRL"}};

    ctk::ScalarPushInput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint"};
    ctk::ScalarPushInput<double> relativePositionSetpoint{this, "relativePositionSetpoint", "", "Relative motor position setpoint"};
    ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]"};
    ctk::ScalarPushInput<int32_t> relativePositionSetpointInSteps{this, "relativePositionSetpointInSteps", "", "Relative motor position setpoint [steps]"};

    //  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "SW_LIM_ENABLE", "", "Enable SW limits", {"CTRL"}};
    //  ctk::ScalarPushInput<double> positiveSWPositionLimit{this, "POS_SW_POS_LIM", "", "Positive SW position limit", {"CTRL"}};
    //  ctk::ScalarPushInput<double> negativeSWPositionLimit{this, "NEG_SW_POS_LIM", "", "Negative SW position limit", {"CTRL"}};

    //FIXME
    ctk::ScalarOutput<double> actualPosition{this, "actualPosition", "", "Actual position [scaled]", {"MEAS"}};

    void prepare() override;
    void mainLoop() override;
  } controlInputs{_motor, this, "ControlInputs", "Control inputs to the stepper motor"};



  // TODO Which values must be passed through MotorControl and what can be piped to the CS directly?
  /** Values read from the motor driver hardware. */
  struct MotorDriverHWReadback : ctk::ApplicationModule {

    MotorDriverHWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    std::shared_ptr<ctk::StepperMotor> _motor;
    ExecutionTimer<> execTimer;

    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};

    ctk::ScalarOutput<int> isCalibrated{this, "isCalibrated", "", "Flag set to true if the motor is calibrated",{"CS", "MOTCTRL"}};
    ctk::ScalarOutput<int32_t> motorErrorId{this, "motorError", "", "Error ID of the motor driver", {"CS"}};
    ctk::ScalarOutput<int> actualPositionInSteps{this, "actualPositionInSteps", "", "Actual position [steps]", {"CS"}};

    ctk::ScalarOutput<float> actualCycleTime{this, "actualCycleTime", "", "Actual cycle time by which the HW is being read", {"CS"}};

    void mainLoop() override;
  } motorDriverHWReadback{_motor, this, "motorDriverHWReadback", "Signals read from the motor driver HW"};


  struct MotorDriverSWReadBack : ctk::ApplicationModule {

    MotorDriverSWReadBack(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

    std::shared_ptr<ctk::StepperMotor> _motor;

    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger to initiate reading from HW"};
    ctk::ScalarOutput<int> isSystemIdle{this, "isSystemIdle", "", "Flags if system is idle and a movement or calibration can be started", {"CS", "MOTCTRL"}};

    void mainLoop() override;
  } motorDriverSWReadback{_motor, this, "motorDriverSWReadback", "Signals read from the motor driver SW"};

};

#endif /* INCLUDE_MOTORDRIVER_H_ */
