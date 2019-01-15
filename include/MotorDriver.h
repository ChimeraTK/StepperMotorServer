/*
 * MotorDriver.h
 *
 *  Created on: Jul 16, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORDRIVER_H_
#define INCLUDE_MOTORDRIVER_H_

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <mtca4u/MotorDriverCard/StepperMotor.h>
//#include <mtca4u/MotorDriverCard/StepperMotorWithReference.h>

#include "ControlInput.h"
#include "MotorDriverReadback.h"

//FIXME Include the dummy in this module or reimplement the library's dummy, so we dont need this

//#include <map>
//#include <functional>
#include <memory>

namespace ctk = ChimeraTK;


//typedef std::map<ctk::TransferElementID, std::function<void(void)>> funcmapT;



struct BasicMotorDriver : public ctk::ModuleGroup {
  BasicMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const ctk::StepperMotorParameters &motorParameters);

  std::shared_ptr<ctk::StepperMotor> motor;
  ControlInputHandler ctrlInputHandler;
  BasicHWReadbackHandler readbackHandler;
};


struct LinearMotorDriver : public ctk::ModuleGroup {

  LinearMotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const ctk::StepperMotorParameters &motorParameters);

  std::shared_ptr<ctk::StepperMotor> motor;
  ControlInputHandler ctrlInputHandler;
  ExtHWReadbackHandler readbackHandler;
};

#endif /* INCLUDE_MOTORDRIVER_H_ */
