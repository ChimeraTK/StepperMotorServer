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

namespace ctk = ChimeraTK;


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
  };




  /**
   * @class HWReadback
   * @details Variables read from the motor driver library (residing in SW)
   * TODO Which values must be passed through MotorControl and what can be piped to the CS directly?
   */
  struct SWReadback : ctk::ApplicationModule {

    SWReadback(std::shared_ptr<ctk::StepperMotor> motor, ctk::EntityOwner *owner, const std::string &name, const std::string &description);

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
  };



#endif /* INCLUDE_MOTORDRIVERREADBACK_H_ */
