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

//typedef std::map<ctk::TransferElementID, std::function<void(void)>> funcmapT;
using funcmapT = std::map<ctk::TransferElementID, std::function<void(void)>>;



struct BasicControlInput : public ctk::VariableGroup {

  BasicControlInput(ctk::EntityOwner *owner, const std::string &name, const std::string &description);
  virtual ~BasicControlInput(){};

  funcmapT funcMap;

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

  virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor);

};


struct LinearMotorControlInput : BasicControlInput {

  LinearMotorControlInput(ctk::EntityOwner *owner, const std::string &name, const std::string &description);


  ctk::ScalarPushInput<int32_t> calibrateMotor{this, "calibrateMotor", "", "Calibrates the motor", {"CS"}};
  ctk::ScalarOutput<int32_t> isPositiveEndSwitchActive{this, "isPositiveEndSwitchActive", "", "Flags if the positive end switch is activated."};

  virtual void createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor);

};

/**
 * @class ControlInputHandler
 * @details General component for handling commands to the motor driver
 */
struct ControlInputHandler : public ctk::ApplicationModule {

  ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description);
  virtual ~ControlInputHandler() {};

  ctk::ReadAnyGroup inputGroup;
  funcmapT _funcMap;
};

/**
 * @class ControlInputHandlerImpl
 * @details Contains the implementation of the ControlInputHandler as a template so it can be used
 *          with a specific motor and set of inputs.
 */
template<typename MotorType, typename ControlInputType>
struct ControlInputHandlerImpl : public ControlInputHandler {

  ControlInputHandlerImpl(ctk::EntityOwner *owner, const std::string &name, const std::string &description, std::shared_ptr<MotorType> motor)
    : ControlInputHandler(owner, name, description),
      _motor(motor),
      _inp{std::make_shared<ControlInputType>(this, "controlInput", "Control inputs")}{};

  ctk::ReadAnyGroup inputGroup;
  funcmapT _funcMap;

  std::shared_ptr<MotorType> _motor;
  std::shared_ptr<ControlInputType> _inp;

  void prepare() override{
      _inp->createFunctionMap(_motor);
  };

  void mainLoop() override{
    inputGroup = this->readAnyGroup();

    // Initialize the motor
    _motor->setActualPositionInSteps(0);

    while(true){

      auto changedVarId = inputGroup.readAny();

      if(_motor->isSystemIdle()
          || changedVarId == _inp->stopMotor.getId() || changedVarId == _inp->emergencyStopMotor.getId()){
        //FIXME Keep this as long as we rely on the dummy for tests
        try{
          _inp->funcMap.at(changedVarId)();
        }
        catch(mtca4u::MotorDriverException &e){
          _inp->userMessage = "WARNING: MotorDriver::ControlInput: Calling motor driver threw an exception: " + std::string(e.what());
        }
        _inp->userMessage = "";
      }
      else{
        _inp->userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
      }

      _inp->dummyMotorStop = _inp->stopMotor || _inp->emergencyStopMotor;
      _inp->dummyMotorTrigger++;

      writeAll();
    }
  };
}; /* struct ControlInputHandlerImpl */

/**
 * @class BasicControlInputHandler
 * @brief Specialization of the ControlInputHandler for basic StepperMotor instances.
 */
using BasicControlInputHandler       = ControlInputHandlerImpl<ctk::StepperMotor, BasicControlInput>;

/**
 * @class LinearMotorControlInputHandler
 * @brief Specialization of the ControlInputHandler for StepperMotorWithReference instances (i.e., linear motors with end switches).
 */
using LinearMotorControlInputHandler = ControlInputHandlerImpl<ctk::StepperMotorWithReference, LinearMotorControlInput>;

#endif /* INCLUDE_CONTROLINPUT_H_ */
