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

#include "ControlInput.h"
#include "MotorDriverReadback.h"

#include <memory>

namespace ctk = ChimeraTK;


struct MotorDriver : public ctk::ModuleGroup {
  MotorDriver(ctk::EntityOwner *owner, const std::string &name, const std::string &description,
                    const ctk::StepperMotorParameters &motorParameters);

  std::shared_ptr<ctk::StepperMotor> motor;
  ControlInputHandler ctrlInputHandler;
  ReadbackHandler readbackHandler;
};


#endif /* INCLUDE_MOTORDRIVER_H_ */
