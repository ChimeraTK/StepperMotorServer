/*
 * MotorCtrl.h
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORCONTROL_H_
#define INCLUDE_MOTORCONTROL_H_

#include <memory>

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>

namespace ctk = ChimeraTK;

struct MotorControl : ctk::ApplicationModule{

  using ctk::ApplicationModule::ApplicationModule;

  //std::shared_ptr<ctk::StepperMotorWithReference> motor;

  enum class MotorState : int {
    MOTOR_READY=0, MOTOR_ERROR=1, MOTOR_RUNNING=2, MOTOR_RESETTING=3, MOTOR_DISABLED=4
  } motorState;

  // Operator inputs TODO Keep these very basic at first
  //                                      Owner, name, unit, descr, tags
  // TODO move to array variants (e.g. ArrayPushInput) to handle multiple motors (MD22)?
  ctk::ScalarPushInput<int32_t> enableMotor{this, "MOTOR_ENABLE", "", "Enable the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> stopMotor{this, "MOTOR_STOP", "", "Stop the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CS"}};

  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "MOTOR_POS_SETP_STEPS", "", "Position setpoint [steps]", {"CS"}};

  // Current status to control system
  ctk::ScalarOutput<double> actualPositionInSteps{this, "ACT_POS_IN_STEPS", "", "Actual position [steps]", {"CS"}};
  ctk::ScalarOutput<int32_t> motorStatus{this, "MOTOR_STATUS", "", "Motor status word", {"CS"}};
  ctk::ScalarOutput<int32_t> motorError{this, "MOTOR_ERROR", "", "Motor error word", {"CS"}};


  void mainLoop() override;

  // Motor states
  void stateMotorDisabled();
  void stateMotorReady();
  void stateMotorRunning();
  void stateMotorError();
  void stateMotorResetting();

}; /* struct MotorControl */
#endif /* INCLUDE_MOTORCONTROL_H_ */
