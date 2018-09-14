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


//FIXME Include the dummy in this module or reimplement the library's dummy, so we dont need this
#include "MotorDriverParameters.h"

#include <map>
#include <functional>
#include <memory>
#include <utility>

namespace ctk = ChimeraTK;

typedef std::map<ctk::TransferElementID, std::function<void(void)>> funcmapT;


struct ControlInput : public ctk::VariableGroup {
  using ctk::VariableGroup::VariableGroup;
  virtual ~ControlInput() {};

  funcmapT funcMap;
  virtual void createFunctionMap()=0;
  funcmapT getFunctionMap(){return funcMap;};
};

struct BasicControlInput : ControlInput {

  BasicControlInput(std::shared_ptr<ctk::StepperMotor>  motor,
                    ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  // This must also know the motor because it defines the function map
  std::shared_ptr<ctk::StepperMotor> _motor;

  ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> stopMotor{this, "stopMotor", "", "Stop the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStopMotor", "", "Emergency stop motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]", {"CS"}};

  //FIXME Move dummy to this module
  ctk::ScalarOutput<int32_t> dummyMotorTrigger{this, "dummyMotorTrigger", "", "Triggers the dummy motor module after writing to a control input", {"DUMMY"}};
  ctk::ScalarOutput<int32_t> dummyMotorStop{this, "dummyMotorStop","", "Stops the dummy motor", {"DUMMY"}};

  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "enableSWPositionLimits", "", "Enable SW limits", {"CS"}};
  ctk::ScalarPushInput<float>   maxSWPositionLimit{this, "maxSWPositionLimit", "", "Positive SW position limit", {"CS"}};
  ctk::ScalarPushInput<float>   minSWPositionLimit{this, "minSWPositionLimit", "", "Negative SW position limit", {"CS"}};
  ctk::ScalarPushInput<int32_t> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "", "Positive SW position limit", {"CS"}};
  ctk::ScalarPushInput<int32_t> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "", "Negative SW position limit", {"CS"}};

  ctk::ScalarOutput<std::string> userMessage{this, "userMessage", "", "Message for user notification from ControlInput module", {"CS"}};

  ctk::ScalarPushInput<int32_t> startMotor{this, "startMotor", "", "Start the motor", {"CS"}};
  ctk::ScalarPushInput<int32_t> enableAutostart{this, "enableAutostart", "", "Sets the autostart flag of the motor driver", {"CS"}};

  void createFunctionMap() override;

};


struct LinearMotorControlInput : BasicControlInput {

  LinearMotorControlInput(std::shared_ptr<ctk::StepperMotorWithReference>  motor,
                                                   ctk::EntityOwner *owner, const std::string &name, const std::string &description);

  std::shared_ptr<ctk::StepperMotorWithReference> _motor;

  ctk::ScalarPushInput<int32_t> calibrateMotor{this, "calibrateMotor", "", "Calibrates the motor", {"CS"}};
  ctk::ScalarOutput<int32_t> isPositiveEndSwitchActive{this, "isPositiveEndSwitchActive", "", "Flags if the positive end switch is activated."};

  void createFunctionMap() override;

};


//struct LinearMotorControlInput : ControlInput {
//
//  LinearMotorControlInput::LinearMotorControlInput(std::shared_ptr<ctk::StepperMotorWithReference>  motor, std::unique_ptr<BasicControlInput> basicInp,
//                                                   ctk::EntityOwner *owner, const std::string &name, const std::string &description);
//
//  std::unique_ptr<BasicControlInput> _basicInput;
//  std::shared_ptr<ctk::StepperMotorWithReference> _motor;
//
//  ctk::ScalarPushInput<int32_t> calibrateMotor{this, "calibrateMotor", "", "Calibrates the motor", {"CS"}};
//  ctk::ScalarOutput<int32_t> isPositiveEndSwitchActive{this, "isPositiveEndSwitchActive", "", "Flags if the positive end switch is activated."};
//
//  void createFunctionMap() override;
//
//};


/**
 * @class ControlInputHandler
 * @details General component for handling commands to the motor driver
 */
struct ControlInputHandler : public ctk::ApplicationModule {

  ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description);
  virtual ~ControlInputHandler() {};

  ctk::ReadAnyGroup inputGroup;
  funcmapT _funcMap;
  //std::unique_ptr<ControlInput> _inp;

//  virtual void prepare();
//  virtual void mainLoop();

//  template <typename MotorType>
//  void mainLoopImpl(std::shared_ptr<MotorType> motor){
//
//    inputGroup = this->readAnyGroup();
//
//    // Initialize the motor
//    motor->setActualPositionInSteps(0);
//
//    while(true){
//
//      auto changedVarId = inputGroup.readAny();
//
//      if(motor->isSystemIdle()
//          || changedVarId == _inp->stopMotor.getId() || changedVarId == _inp->emergencyStopMotor.getId()){
//        _inp->funcMap.at(changedVarId)();
//        _inp->userMessage = "";
//      }
//      else{
//        _inp->userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
//      }
//
//      _inp->dummyMotorStop = _inp->stopMotor || _inp->emergencyStopMotor;
//      _inp->dummyMotorTrigger++;
//
//      writeAll();
//    }
//  }
};

/**
   * @class BasicControlInput
   * @details This module manages basic control input corresponding to the StepperMotor object provided by the motor driver library
   */
struct BasicControlInputHandler : ControlInputHandler {

  BasicControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description, std::shared_ptr<ctk::StepperMotor> motor)
      : ControlInputHandler(owner, name, description), _motor(motor),
        _inp{_motor, this, "controlInput", "Control inputs"} {};

  BasicControlInput _inp;
  std::shared_ptr<ctk::StepperMotor> _motor;

  //ctk::ScalarPollInput<int32_t> systemIdle{this, "systemIdle", "", "System idle flag"};

//    TODO Equivalent reset method?
//  ctk::ScalarPushInput<int32_t> enableMotor{this, "enable", "", "Enable the motor", {"CS"}};
//  ctk::ScalarPushInput<int32_t> stopMotor{this, "stopMotor", "", "Stop the motor", {"CS"}};
//  ctk::ScalarPushInput<int32_t> emergencyStopMotor{this, "emergencyStopMotor", "", "Emergency stop motor", {"CS"}};
//  ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]", {"CS"}};
//
//  //FIXME Move dummy to this module
//  ctk::ScalarOutput<int32_t> dummyMotorTrigger{this, "dummyMotorTrigger", "", "Triggers the dummy motor module after writing to a control input"};
//  ctk::ScalarOutput<int32_t> dummyMotorStop{this, "dummyMotorStop","", "Stops the dummy motor"};
//
//  ctk::ScalarPushInput<int32_t> enableSWPositionLimits{this, "enableSWPositionLimits", "", "Enable SW limits", {"CS"}};
//  ctk::ScalarPushInput<float>   maxSWPositionLimit{this, "maxSWPositionLimit", "", "Positive SW position limit", {"CS"}};
//  ctk::ScalarPushInput<float>   minSWPositionLimit{this, "minSWPositionLimit", "", "Negative SW position limit", {"CS"}};
//  ctk::ScalarPushInput<int32_t> maxSWPositionLimitInSteps{this, "maxSWPositionLimitInSteps", "", "Positive SW position limit", {"CS"}};
//  ctk::ScalarPushInput<int32_t> minSWPositionLimitInSteps{this, "minSWPositionLimitInSteps", "", "Negative SW position limit", {"CS"}};
//
//  ctk::ScalarOutput<std::string> userMessage{this, "userMessage", "", "Message for user notification from ControlInput module", {"CS"}};
//
//  ctk::ScalarPushInput<int32_t> startMotor{this, "startMotor", "", "Start the motor", {"CS"}};
//  ctk::ScalarPushInput<int32_t> enableAutostart{this, "enableAutostart", "", "Sets the autostart flag of the motor driver", {"CS"}};


  //ctk::ScalarPushInput<int32_t> startMotorRelative{this, "MOTOR_START_REL", "", "Start relative movement of motor", {"CTRL"}};
  //ctk::ScalarPushInput<int32_t> resetMotor{this, "MOTOR_RESET", "", "Reset the motor", {"CTRL"}};

//    ctk::ScalarPushInput<double> positionSetpoint{this, "positionSetpoint", "", "Motor position setpoint", {"CS"}};
//    ctk::ScalarPushInput<double> relativePositionSetpoint{this, "relativePositionSetpoint", "", "Relative motor position setpoint", {"CS"}};
//    ctk::ScalarPushInput<int32_t> positionSetpointInSteps{this, "positionSetpointInSteps", "", "Motor position setpoint [steps]", {"CS"}};
//    ctk::ScalarPushInput<int32_t> relativePositionSetpointInSteps{this, "relativePositionSetpointInSteps", "", "Relative motor position setpoint [steps]", {"CS"}};


  void prepare() override;
  void mainLoop() override;

};


/**
   * @class LinearMotorControlInput
   * @details This module decorates the BasicControlInput component in order to add functionality of
   *          the StepperMotorWithReference object of the motor driver library
   */
struct LinearMotorControlInputHandler : ControlInputHandler {

  LinearMotorControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                                 std::shared_ptr<ctk::StepperMotorWithReference> motor)
      : ControlInputHandler{owner, name, description},
        _motor{motor},
        _inp{_motor, /* std::unique_ptr<BasicControlInput>{new BasicControlInput{motor, this, "basicControlInput", ""}} */ this, "controlInput", "Linear motor control inputs"}{};

  LinearMotorControlInput _inp;
  std::shared_ptr<ctk::StepperMotorWithReference> _motor;

  void prepare() override;
  void mainLoop() override;
};



#endif /* INCLUDE_CONTROLINPUT_H_ */
