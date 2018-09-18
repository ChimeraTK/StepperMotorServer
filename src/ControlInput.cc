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


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
BasicControlInput::BasicControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ctk::VariableGroup{owner, name, description, true} {};

void BasicControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotor> _motor){

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
}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
LinearMotorControlInput::LinearMotorControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : BasicControlInput{owner, name, description} {};

void LinearMotorControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor){

  BasicControlInput::createFunctionMap(_motor);

  funcMap[calibrateMotor.getId()] = [this, _motor]{_motor->calibrate(); isPositiveEndSwitchActive = _motor->isPositiveReferenceActive(); };

}
