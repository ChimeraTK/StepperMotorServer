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


//FIXME Include the dummy in this module or reimplement the library's dummy, so we dont need this
#include "MotorDriverParameters.h"

#include <map>
#include <functional>
#include <memory>
#include <utility>

namespace ctk = ChimeraTK;


using funcmapT = std::map<ctk::TransferElementID, std::function<void(void)>>;

template <typename UserType>
class EdgeDetector {
  UserType _oldValue;

public:
  EdgeDetector(UserType oldValue = 0)
    : _oldValue{oldValue}{};

  bool isPositiveEdge(UserType signal){
    if(signal && !_oldValue){
      _oldValue = signal;
      return true;
    }
    else{
      return false;
    }
  };
};


struct BasicControlInput : public ctk::VariableGroup {

  BasicControlInput(ctk::EntityOwner *owner, const std::string &name, const std::string &description);
  virtual ~BasicControlInput(){};

  ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> stopMotor{this, "stopMotor", "", "Stop the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStopMotor", "", "Emergency stop motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "steps", "Motor position setpoint", {"CS"}};
  ctk::ScalarPushInput<float>   positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint [user-defined unit]", {"CS"}};

  ctk::ScalarPushInput<int32_t> startMotor{this, "startMotor", "", "Start the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> enableAutostart{this, "enableAutostart", "", "Sets the autostart flag of the motor driver", {"CS"}};

  ctk::ScalarPushInput<int32_t> moveRelativeInSteps{this, "moveRelativeInSteps", "",
                                                    "Initiates a movement relative to the current position. Receives the position change in steps.", {"CS"}};
  ctk::ScalarPushInput<float>   moveRelative{this, "moveRelative", "",
                                             "Initiates a movement relative to the current position. Receives the position change in the user-defined unit.", {"CS"}};

  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "enableSWPositionLimits", "", "Enable SW limits", {"CS"}};
  ctk::ScalarPushInput<float>   maxSWPositionLimit{this, "maxSWPositionLimit", "", "Positive SW position limit", {"CS"}};
  ctk::ScalarPushInput<float>   minSWPositionLimit{this, "minSWPositionLimit", "", "Negative SW position limit", {"CS"}};
  ctk::ScalarPushInput<int32_t> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "", "Positive SW position limit", {"CS"}};
  ctk::ScalarPushInput<int32_t> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "", "Negative SW position limit", {"CS"}};

  ctk::ScalarPushInput<double> currentLimit{this, "currentLimit", "A", "User current limit for the motor", {"CS"}};
  ctk::ScalarPushInput<double> speedLimit{this, "speedLimit", "", "User speed limit for the motor", {"CS"}};

  ctk::ScalarPushInput<int32_t> enableFullStepping{this, "", "Enables full-stepping mode of the motor driver, i.e., it will only stop on full steps", {"CS"}};

  // Message output for feedback to the user
  ctk::ScalarOutput<std::string> userMessage{this, "userMessage", "", "Message for user notification from ControlInput module", {"CS"}};

  //FIXME Move dummy to this module
  ctk::ScalarOutput<int32_t> dummyMotorTrigger{this, "dummyMotorTrigger", "", "Triggers the dummy motor module after writing to a control input", {"DUMMY"}};
  ctk::ScalarOutput<int32_t> dummyMotorStop{this, "dummyMotorStop","", "Stops the dummy motor", {"DUMMY"}};

  //virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor);

};


struct LinearMotorControlInput : public ctk::VariableGroup {

  LinearMotorControlInput(ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  ctk::ScalarPushInput<int32_t> calibrateMotor{this, "calibrateMotor", "", "Calibrates the motor", {"CS"}};
};


/**
 *  @class ControlInputHandlerImpl
 *  @details Contains the implementation of the ControlInputHandler as a template so it can be used
 *           with a specific motor and set of inputs.
 */
class BasicControlInputHandler : public ctk::ApplicationModule {
public:

  BasicControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description, std::shared_ptr<ctk::StepperMotor> motor)
    : ctk::ApplicationModule(owner, name, description),
      _motor(motor),
      _controlInput{this, "controlInput", "Basic control inputs"}{};

  virtual ~BasicControlInputHandler() {};

  virtual void prepare() override{
      createFunctionMap(_motor);
  };

  virtual void mainLoop() override;

protected:
  virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor);
  funcmapT funcMap;

  // Callbacks
  void enableCallback();
  void startCallback();
  void setTargetPositionCallback();
  void setTargetPositionInStepsCallback();

private:
  ctk::ReadAnyGroup inputGroup;
  std::shared_ptr<ctk::StepperMotor> _motor;
  BasicControlInput _controlInput;

}; /* class BasicControlInputHandler */


/**
 * @class LinearMotorControlInputHandler
 * @brief Specialization of the ControlInputHandler for StepperMotorWithReference instances (i.e., linear motors with end switches).
 */
class LinearMotorControlInputHandler : public BasicControlInputHandler{
public:
  LinearMotorControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                                                 std::shared_ptr<ctk::StepperMotorWithReference> motor)
    : BasicControlInputHandler(owner, name, description, motor),
      _motor{motor},
      _controlInput{this, "linearMotorControlInput", "Control inputs for a linear stepper motor with reference switches"}{};

  virtual ~LinearMotorControlInputHandler(){};

  virtual void prepare() override{
    BasicControlInputHandler::createFunctionMap(_motor);
    createFunctionMap(_motor);
  };

protected:
  virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor);

private:
  std::shared_ptr<ctk::StepperMotorWithReference> _motor;
  LinearMotorControlInput _controlInput;
};

#endif /* INCLUDE_CONTROLINPUT_H_ */