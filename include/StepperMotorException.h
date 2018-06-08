/*
 * StepperMotorException.h
 *
 *  Created on: Jul 21, 2016
 *      Author: vitimic
 */
//TODO Keep this module? More expressive exceptions?

#include <exception>
#include <string>

#ifndef STEPPER_MOTOR_EXCEPTION_H_
#define STEPPER_MOTOR_EXCEPTION_H_

#define EXCEPTION_MESSAGE std::string("error in ") + std::string(__PRETTY_FUNCTION__)

class StepperMotorException : public std::exception{
public:
  StepperMotorException(std::string what);
  virtual const char* what() const throw();
  virtual ~StepperMotorException() throw();
protected:
  std::string _what;
};

#endif /* STEPPER_MOTOR_EXCEPTION_H_ */
