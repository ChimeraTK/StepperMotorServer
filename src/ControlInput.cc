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

void ControlInputHandler::mainLoopImpl(std::shared_ptr<BasicControlInput> _inp, std::shared_ptr<ctk::StepperMotor> motor){

  inputGroup = this->readAnyGroup();

  // Initialize the motor
  motor->setActualPositionInSteps(0);

  while(true){

    auto changedVarId = inputGroup.readAny();

    if(motor->isSystemIdle()
        || changedVarId == _inp->stopMotor.getId() || changedVarId == _inp->emergencyStopMotor.getId()){
      _inp->funcMap.at(changedVarId)();
      _inp->userMessage = "";
    }
    else{
      _inp->userMessage = "WARNING: MotorDriver::ControlInput: Illegal write attempt while motor is not in IDLE state.";
    }

    _inp->dummyMotorStop = _inp->stopMotor || _inp->emergencyStopMotor;
    _inp->dummyMotorTrigger++;

    writeAll();
  }
}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
BasicControlInput::BasicControlInput(std::shared_ptr<ctk::StepperMotor>  motor,
                                     ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ctk::VariableGroup{owner, name, description, true} /*ControlInput{owner, name, description, true},
    _motor(motor)*/ {};

void BasicControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor){

  std::cout << "    *** Motor use count: " << _motor.use_count() << std::endl;

  funcMap[enableMotor.getId()]               = [this, _motor]{/*std::cout << "Hit enabled " << enableMotor <<std::endl;*/ _motor->setEnabled(enableMotor); };
  funcMap[positionSetpointInSteps.getId()]   = [this, _motor]{ _motor->setTargetPositionInSteps(positionSetpointInSteps); };
  funcMap[startMotor.getId()]                = [this, _motor]{ _motor->start();};
  funcMap[stopMotor.getId()]                 = [this, _motor]{ if(stopMotor){_motor->stop();}};
  funcMap[emergencyStopMotor.getId()]        = [this, _motor]{ if(emergencyStopMotor){ _motor->emergencyStop();}};
  funcMap[enableAutostart.getId()]           = [this, _motor]{ _motor->setAutostart(enableAutostart);};
  funcMap[enableSWPositionLimits.getId()]    = [this, _motor]{ _motor->setSoftwareLimitsEnabled(enableSWPositionLimits); };
  funcMap[maxSWPositionLimit.getId()]        = [this, _motor]{ _motor->setMaxPositionLimit(maxSWPositionLimit); };
  funcMap[minSWPositionLimit.getId()]        = [this, _motor]{ _motor->setMinPositionLimit(minSWPositionLimit); };
  funcMap[maxSWPositionLimitInSteps.getId()] = [this, _motor]{ _motor->setMaxPositionLimitInSteps(maxSWPositionLimitInSteps); };
  funcMap[minSWPositionLimitInSteps.getId()] = [this, _motor]{ _motor->setMinPositionLimitInSteps(minSWPositionLimitInSteps); };

  std::cout << "Basic Motor function map size: " << funcMap.size() << std::endl;
  std::cout << "    *** Motor use count: " << _motor.use_count() << std::endl;
}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
LinearMotorControlInput::LinearMotorControlInput(std::shared_ptr<ctk::StepperMotorWithReference>  motor,
                                     ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : BasicControlInput{motor, owner, name, description} {};

void LinearMotorControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor){

  BasicControlInput::createFunctionMap(_motor);

  funcMap[calibrateMotor.getId()] = [this, _motor]{ std::cout << "    ** Called calibrate"<< std::endl; _motor->calibrate(); isPositiveEndSwitchActive = _motor->isPositiveReferenceActive(); };

  std::cout << "Linear Motor function map size: " << funcMap.size() << std::endl;
}



//void ControlInputHandler::prepare(){
//  _inp.createFunctionMap();
////  _funcMap = _inp.getFunctionMap();
//}


void BasicControlInputHandler::prepare(){
  _inp->createFunctionMap(_motor);
}

void BasicControlInputHandler::mainLoop(){

  mainLoopImpl(_inp, _motor);

}


void LinearMotorControlInputHandler::prepare(){
  _inp->createFunctionMap(_motor);
}

void LinearMotorControlInputHandler::mainLoop(){

  mainLoopImpl(_inp, static_cast<std::shared_ptr<ctk::StepperMotor>>(_motor));

}

