/*
 * ControlInput.h
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_CONTROLINPUT_H_
#define INCLUDE_CONTROLINPUT_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <mtca4u/MotorDriverCard/StepperMotor.h>
#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>
#include <mtca4u/MotorDriverCard/MotorDriverException.h>

#include <map>
#include <functional>
#include <memory>
#include <utility>

namespace ctk = ChimeraTK;

/**
 * A map between the TransferElementID of a PV and the associated
 * function of the MotorDriverCard library. This allows to pass on
 * the changed PV to the library by the ID returned from readAny().
 */
using funcmapT = std::map<ctk::TransferElementID, std::function<void(void)>>;


/// Motor control data
struct MotorControl : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarPushInput<int> enable{this, "enable", "", "Enable the motor"};
  ctk::ScalarPushInput<int> disable{this, "disable", "", "Disable the motor"};
  ctk::ScalarPushInput<int> stop{this, "stop", "", "Stop the motor"};
  ctk::ScalarPushInput<int> emergencyStop{this, "emergencyStop", "", "Emergency stop motor"};
  ctk::ScalarPushInput<int> resetError{this, "resetError", "", "Reset error state"};

  ctk::ScalarPushInput<int> enableFullStepping{this, "enableFullStepping", "", "Enables full-stepping mode of the motor driver, i.e., it will only stop on full steps"};
  ctk::ScalarPushInput<int> enableAutostart{this, "enableAutostart", "", "Sets the autostart flag of the motor driver"};
};

struct SoftwareLimitCtrl : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarPushInput<int> enable{this, "enable", "", "Enable SW limits"};
  ctk::ScalarPushInput<float> maxPosition{this, "maxPosition", "", "Positive SW position limit"};
  ctk::ScalarPushInput<float> minPosition{this, "minPosition", "", "Negative SW position limit"};
  ctk::ScalarPushInput<int> maxPositionInSteps{this, "maxPositionInSteps", "", "Positive SW position limit"};
  ctk::ScalarPushInput<int> minPositionInSteps{this, "minPositionInSteps", "", "Negative SW position limit"};
};


struct ControlInput : public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;


  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "steps", "Motor position setpoint", {"MOTOR"}};
  ctk::ScalarPushInput<float>   positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint [user-defined unit]", {"MOTOR"}};

  ctk::ScalarPushInput<int32_t> startMotor{this, "startMotor", "", "Start the motor", {"MOTOR"}};

  ctk::ScalarPushInput<int32_t> moveRelativeInSteps{this, "moveRelativeInSteps", "",
                                                    "Initiates a movement relative to the current position. Receives the position change in steps.", {"MOTOR"}};
  ctk::ScalarPushInput<float>   moveRelative{this, "moveRelative", "",
                                             "Initiates a movement relative to the current position. Receives the position change in the user-defined unit.", {"MOTOR"}};

  ctk::ScalarPushInput<float>   referencePosition{this, "referencePosition", "", "Writing to this value sets the actual motor position to a given reference", {"MOTOR"}};
  ctk::ScalarPushInput<int32_t> referencePositionInSteps{this, "referencePositionInSteps", "", "Writing to this value sets the actual motor position to a given reference", {"MOTOR"}};

  ctk::ScalarPushInput<int32_t> encoderReferencePosition{this, "encoderReferencePosition", "", "Writing to this value sets the actual encoder position to a given reference", {"MOTOR"}};

  ctk::ScalarPushInput<int32_t> axisTranslationInSteps{this, "axisTranslationInSteps", "steps", "Offset to translate axis, i.e. shift the reference point.", {"MOTOR"}};
  ctk::ScalarPushInput<float>   axisTranslation{this, "axisTranslation", "", "Offset to translate axis, i.e. shift the reference point.", {"MOTOR"}};



  ctk::ScalarPushInput<double> currentLimit{this, "currentLimit", "A", "User current limit for the motor", {"MOTOR"}};
  ctk::ScalarPushInput<double> speedLimit{this, "speedLimit", "", "User speed limit for the motor", {"MOTOR"}};


  // Message output for feedback to the user
  ctk::ScalarOutput<int32_t>        userWarning{this, "userWarning", "", "Warning flag, true when an invalid input has been issued.", {"MOTOR"}};
  ctk::ScalarOutput<std::string> userMessage{this, "userMessage", "", "Message for user notification from ControlInput module", {"MOTOR"}};

  // Values triggering the dummy if needed
  ctk::ScalarOutput<int32_t> dummyMotorTrigger{this, "dummyMotorTrigger", "", "Triggers the dummy motor module after writing to a control input", {"DUMMY"}};
  ctk::ScalarOutput<int32_t> dummyMotorStop{this, "dummyMotorStop","", "Stops the dummy motor", {"DUMMY"}};

};


struct CalibrationCommands: public ctk::VariableGroup {

  using ctk::VariableGroup::VariableGroup;

  ctk::ScalarPushInput<int32_t> calibrateMotor{this, "calibrateMotor", "", "Calibrates the motor", {"MOTOR"}};
  ctk::ScalarPushInput<int32_t> determineTolerance{this, "determineTolerance", "", "Determines tolerance of the end switch positions", {"MOTOR"}};
};


/**
 *  @class ControlInputHandlerImpl
 *  @details Contains the implementation of the ControlInputHandler as a template so it can be used
 *           with a specific motor and set of inputs.
 */
class ControlInputHandler : public ctk::ApplicationModule {
public:

  ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description, std::shared_ptr<ctk::StepperMotor> motor);

  //virtual ~ControlInputHandler() {}

  virtual void prepare() override;
  virtual void mainLoop() override;

private:
  virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor);
  virtual void appendCalibrationToMap();
  funcmapT funcMap;
  ControlInput _controlInput;
  MotorControl control{this, "control", "Control words of the motor", false, {"MOTOR"}};
  SoftwareLimitCtrl swLimits{this, "swLimits", "Control data of SW limits", false, {"MOTOR"}};
  CalibrationCommands _calibrationCommands;

  // Callbacks for the BasiStepperMotor
  void enableCallback();
  void disableCallback();
  void startCallback();
  void setTargetPositionCallback();
  void setTargetPositionInStepsCallback();

  //Callbacks for the LinearStepperMotor
  void calibrateCallback();
  void determineToleranceCallback();

  ctk::ReadAnyGroup inputGroup;
  std::shared_ptr<ctk::StepperMotor> _motor;

}; /* class ControlInputHandler */
#endif /* INCLUDE_CONTROLINPUT_H_ */
