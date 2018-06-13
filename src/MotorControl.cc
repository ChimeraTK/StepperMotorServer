/*
 * MotorControl.cc
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#include "MotorControl.h"
#include <iostream>

void MotorControl::mainLoop(){

  // TODO Implement state machine in EqFct of BAMMotor, or improve it
  motorState = MotorState::MOTOR_DISABLED;

  while(true){

    switch (motorState){
      case MotorState::MOTOR_DISABLED:
        stateMotorDisabled();
        break;
      case MotorState::MOTOR_READY:
        stateMotorReady();
        break;
      case MotorState::MOTOR_RUNNING:
        stateMotorRunning();
        break;
      case MotorState::MOTOR_RESETTING:
        stateMotorResetting();
        break;
      case MotorState::MOTOR_ERROR:
        stateMotorError();
        break;
    }

  }
}/* mainLoop() */

void MotorControl::stateMotorDisabled(){
  std::cout << "Entered state MotorDisabled." << std::endl;
  while(true){
    readAny();
    if(enableMotor != 0){
      motorState = MotorState::MOTOR_READY;
      return;
    }
  }
}

void MotorControl::stateMotorReady(){
  std::cout << "Entered state MotorReady." << std::endl;
  while(true){
    readAll();
    if(startMotor != 0 && positionSetpointInSteps != actualPositionInSteps){
      // TODO initiate movement here
      actualPositionInSteps = positionSetpointInSteps;
      motorState = MotorState::MOTOR_RUNNING;
      return;
    }
  }
}

void MotorControl::stateMotorRunning(){
  std::cout << "Entered state MotorRunning." << std::endl;
  //TODO Implement
  motorState = MotorState::MOTOR_DISABLED;
  return;
}

void MotorControl::stateMotorResetting(){
  std::cout << "Entered state MotorResetting." << std::endl;
  //TODO Implement
  motorState = MotorState::MOTOR_DISABLED;
  return;
}

void MotorControl::stateMotorError(){
  std::cout << "Entered state MotorError." << std::endl;
  //TODO Implement
  motorState = MotorState::MOTOR_DISABLED;
  return;
}




