/*
 * ControlInput.cc
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#include "ControlInput.h"


// Definitions of ControlInputHandler module
ControlInputHandler::ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description, true) {}


// Specialize templates for the supported motor types
//template void ControlInputHandler::mainLoopImpl<ctk::StepperMotor>(std::shared_ptr<ctk::StepperMotor>){};
//template void ControlInputHandler::mainLoopImpl<ctk::StepperMotorWithReference>(std::shared_ptr<ctk::StepperMotorWithReference>){};

/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
BasicControlInput::BasicControlInput(std::shared_ptr<ctk::StepperMotor>  motor,
                                     ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ControlInput{owner, name, description, true},
    _motor(motor) {};

void BasicControlInput::createFunctionMap(){

  funcMap[enableMotor.getId()]               = [this](){std::cout << "Hit enabled " << enableMotor <<std::endl;/* _motor->setEnabled(enableMotor);*/ };
  funcMap[positionSetpointInSteps.getId()]   = [this](){ _motor->setTargetPositionInSteps(positionSetpointInSteps); };
  funcMap[startMotor.getId()]                = [this](){ _motor->start();};
  funcMap[stopMotor.getId()]                 = [this](){ if(stopMotor){_motor->stop();}};
  funcMap[emergencyStopMotor.getId()]        = [this](){ if(emergencyStopMotor){ _motor->emergencyStop();}};
  funcMap[enableAutostart.getId()]           = [this](){ _motor->setAutostart(enableAutostart);};
  funcMap[enableSWPositionLimits.getId()]    = [this](){ _motor->setSoftwareLimitsEnabled(enableSWPositionLimits); };
  funcMap[maxSWPositionLimit.getId()]        = [this](){ _motor->setMaxPositionLimit(maxSWPositionLimit); };
  funcMap[minSWPositionLimit.getId()]        = [this](){ _motor->setMinPositionLimit(minSWPositionLimit); };
  funcMap[maxSWPositionLimitInSteps.getId()] = [this](){ _motor->setMaxPositionLimitInSteps(maxSWPositionLimitInSteps); };
  funcMap[minSWPositionLimitInSteps.getId()] = [this](){ _motor->setMinPositionLimitInSteps(minSWPositionLimitInSteps); };

  std::cout << "Basic Motor function map size: " << funcMap.size() << std::endl;
}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
LinearMotorControlInput::LinearMotorControlInput(std::shared_ptr<ctk::StepperMotorWithReference>  motor,
                                     ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  /*: ControlInput{owner, name, description, true},*/
  : _motor{motor},
    BasicControlInput{_motor, owner, "basicControlInput", "Control input for basic StepperMotor functionality"} {};

void LinearMotorControlInput::createFunctionMap(){

  BasicControlInput::createFunctionMap();

  funcMap[calibrateMotor.getId()] = [this](){ _motor->calibrate(); isPositiveEndSwitchActive = _motor->isPositiveReferenceActive(); };

  std::cout << "Linear Motor function map size: " << funcMap.size() << std::endl;
}


///* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
//LinearMotorControlInput::LinearMotorControlInput(std::shared_ptr<ctk::StepperMotorWithReference>  motor, std::unique_ptr<BasicControlInput> basicInp,
//                                     ctk::EntityOwner *owner,
//                                     const std::string &name,
//                                     const std::string &description)
//  : ControlInput{owner, name, description, true},
//    _basicInput{new BasicControlInput{motor, owner, "basicControlInput", "Control input for basic StepperMotor functionality"}},
//    _motor{motor} {};
//
//void LinearMotorControlInput::createFunctionMap(){
//
//  _basicInput->createFunctionMap();
//  funcMap = _basicInput->funcMap;
//
//  funcMap[calibrateMotor.getId()] = [this](){ _motor->calibrate(); isPositiveEndSwitchActive = _motor->isPositiveReferenceActive(); };
//}


//void ControlInputHandler::prepare(){
//  _inp.createFunctionMap();
////  _funcMap = _inp.getFunctionMap();
//}


void BasicControlInputHandler::prepare(){
  _inp.createFunctionMap();
}

void BasicControlInputHandler::mainLoop(){

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  std::cout << "Basic ctrl handler mainLoop entered." << std::endl;
  while(true){

    auto changedVarId = inputGroup.readAny();

    std::cout << "Basic ctrl handler mainLoop received update." << std::endl;

    if(_motor->isSystemIdle()
        || changedVarId == _inp.stopMotor.getId() || changedVarId == _inp.emergencyStopMotor.getId()){
      _inp.funcMap.at(changedVarId)();
      _inp.userMessage = "";
    }
    else{
      _inp.userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
    }

    _inp.dummyMotorStop = _inp.stopMotor || _inp.emergencyStopMotor;
    _inp.dummyMotorTrigger++;

    writeAll();
  }
}


void LinearMotorControlInputHandler::prepare(){
  _inp.createFunctionMap();
}

void LinearMotorControlInputHandler::mainLoop(){

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  _motor->setActualPositionInSteps(0);

  std::cout << "Linear motor ctrl handler mainLoop entered." << std::endl;
  while(true){

    auto changedVarId = inputGroup.readAny();

    std::cout << "Linear motor ctrl handler mainLoop received update." << std::endl;

    if(_motor->isSystemIdle()
        || changedVarId == _inp.stopMotor.getId() || changedVarId == _inp.emergencyStopMotor.getId()){
      _inp.funcMap.at(changedVarId)();
      _inp.userMessage = "";
    }
    else{
      _inp.userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
    }

    _inp.dummyMotorStop = _inp.stopMotor || _inp.emergencyStopMotor;
    _inp.dummyMotorTrigger++;

    writeAll();
  }
}


