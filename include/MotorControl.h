/*
 * MotorCtrl.h
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORCONTROL_H_
#define INCLUDE_MOTORCONTROL_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>

namespace ctk = ChimeraTK;

struct MotorControl : ctk::ApplicationModule{

  using ctk::ApplicationModule::ApplicationModule;

  //Trigger for cyclic update
  ctk::ScalarPushInput<int> trigger{this, };

  // Operator inputs TODO Keep these very basic at first
  //                                      Owner, name, unit, descr, tags
  // TODO move to array variants (e.g. ArrayPushInput) to handle multiple motors (MD22)?
  ctk::ScalarPushInput<int32_t> startMotor{this, "MOTOR_START", "", "Start the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> stopMotor{this, "MOTOR_STOP", "", "Stop the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CS"}};

  // Current status to control system
  ctk::ScalarOutput<double> actualPositionInSteps{this, "ACT_POS_IN_STEPS", "", "Actual position [steps]", {"CS"}};
  ctk::ScalarOutput<int32_t> motorStatus{this, "MOTOR_STATUS", "", "Motor status word", {"CS"}};
  ctk::ScalarOutput<int32_t> motorError{this, "MOTOR_ERROR", "", "Motor error word", {"CS"}};


  void mainLoop() override;

} /* struct MotorControl */



#endif /* INCLUDE_MOTORCONTROL_H_ */
