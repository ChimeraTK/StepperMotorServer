/*
 * ControlInput.cc
 *
 *  Created on: Sep 13, 2018
 *      Author: ckampm
 */

#include "ControlInput.h"


// Definitions of ControlInputHandler module
ControlInputHandler::ControlInputHandler(ctk::EntityOwner *owner, const std::string &name, const std::string &description)
  : ctk::ApplicationModule(owner, name, description) {}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
BasicControlInput::BasicControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : ctk::VariableGroup{owner, name, description, true} {};

void BasicControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotor> motor){

  funcMap[enableMotor.getId()]               = [this, motor]{ motor->setEnabled(enableMotor); };
  funcMap[positionSetpointInSteps.getId()]   = [this, motor]{ motor->setTargetPositionInSteps(positionSetpointInSteps); };
  funcMap[positionSetpoint.getId()]          = [this, motor]{ motor->setTargetPosition(positionSetpoint); };
  funcMap[startMotor.getId()]                = [this, motor]{ if(startMotor){motor->start();} };
  funcMap[stopMotor.getId()]                 = [this, motor]{ if(stopMotor){motor->stop();} };
  funcMap[emergencyStopMotor.getId()]        = [this, motor]{ if(emergencyStopMotor){ motor->emergencyStop();} };
  funcMap[enableAutostart.getId()]           = [this, motor]{ motor->setAutostart(enableAutostart);};
  funcMap[moveRelativeInSteps.getId()]       = [this, motor]{ motor->moveRelativeInSteps(moveRelativeInSteps); };
  funcMap[moveRelative.getId()]              = [this, motor]{ motor->moveRelative(moveRelative); };
  funcMap[enableSWPositionLimits.getId()]    = [this, motor]{ motor->setSoftwareLimitsEnabled(enableSWPositionLimits); };
  funcMap[maxSWPositionLimit.getId()]        = [this, motor]{ motor->setMaxPositionLimit(maxSWPositionLimit); };
  funcMap[minSWPositionLimit.getId()]        = [this, motor]{ motor->setMinPositionLimit(minSWPositionLimit); };
  funcMap[maxSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMaxPositionLimitInSteps(maxSWPositionLimitInSteps); };
  funcMap[minSWPositionLimitInSteps.getId()] = [this, motor]{ motor->setMinPositionLimitInSteps(minSWPositionLimitInSteps); };
  funcMap[currentLimit.getId()]              = [this, motor]{ motor->setUserCurrentLimit(currentLimit); };
  funcMap[speedLimit.getId()]                = [this, motor]{ motor->setUserSpeedLimit(currentLimit); };
  funcMap[enableFullStepping.getId()]        = [this, motor]{ motor->enableFullStepping(); };
}


/* eliminateHierarchy set to true because we have this group to handle motor features, not to create additional structure on the PVs */
LinearMotorControlInput::LinearMotorControlInput(ctk::EntityOwner *owner,
                                     const std::string &name,
                                     const std::string &description)
  : BasicControlInput{owner, name, description} {};

void LinearMotorControlInput::createFunctionMap(std::shared_ptr<ctk::StepperMotorWithReference> _motor){

  BasicControlInput::createFunctionMap(_motor);

  funcMap[calibrateMotor.getId()] = [this, _motor]{_motor->calibrate();};

}
