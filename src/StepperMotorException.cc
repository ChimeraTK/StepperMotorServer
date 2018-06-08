/*
 * StepperMotorException.cc
 *
 *  Created on: Jul 21, 2016
 *      Author: vitimic
 */

#include "StepperMotorException.h"

StepperMotorException::StepperMotorException(std::string what) : std::exception(), _what(what) {}

StepperMotorException::~StepperMotorException() throw() {}

const char* StepperMotorException::what() const throw(){
  return _what.c_str();
}



