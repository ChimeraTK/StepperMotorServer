/*
 * MotorControl.cc
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#include "MotorControl.h"
#include <iostream>


void MotorControl::mainLoop(){

  inputGroup = readAnyGroup();

  std::cout << "  *** Starting MotorControl mainLoop..." << std::endl;

  // TODO Implement state machine in EqFct of BAMMotor, or improve it
  motorState = MotorState::MOTOR_DISABLED;

  while(true){

    //TODO Read Status/Error words

    std::cout << "Current state is: " << static_cast<int>(motorState) << std::endl;

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
    //FIXME: just to debug
    motorStatus = static_cast<int>(motorState);
    motorStatus.write();

    currentPosition.read();
    double cp = currentPosition;
    actualPosition = cp;
    actualPosition.write();

  }
}/* mainLoop() */

void MotorControl::stateMotorDisabled(){
  std::cout << "Entered state MotorDisabled." << std::endl;
  while(true){
    inputGroup.readAny();
    if(enableMotor != 0){
      toMotorDriver.enableMotor = 1;
      toMotorDriver.enableMotor.write();
      motorState = MotorState::MOTOR_READY;
      return;
    }
  }
}

void MotorControl::stateMotorReady(){
  std::cout << "Entered state MotorReady." << std::endl;
  while(true){
    inputGroup.readAny();
    if(enableMotor == 0){
      toMotorDriver.enableMotor = 0;
      toMotorDriver.enableMotor.write();
      motorState = MotorState::MOTOR_DISABLED;
      return;
    }
    else if(startMotor != 0 && positionSetpointInSteps != actualPositionInSteps && isCalibrated != 0){
      // TODO initiate movement here
      actualPositionInSteps = positionSetpointInSteps;
      toMotorDriver.positionSetpoint = positionSetpointInSteps;
      toMotorDriver.positionSetpoint.write();
      actualPositionInSteps.write();

      motorState = MotorState::MOTOR_RUNNING;
      return;
    }
  }
}

void MotorControl::stateMotorRunning(){
  std::cout << "Entered state MotorRunning." << std::endl;
  while(true){
    auto id = inputGroup.readAny();
    if(stopMotor != 0){
      toMotorDriver.stopMotor = 1;
      toMotorDriver.stopMotor.write();
      //TODO Implement checking of motor.isBusy()
      motorState = MotorState::MOTOR_READY;
      return;
    }
    if(id == isSystemIdle.getId() && !isSystemIdle){
      motorState = MotorState::MOTOR_READY;
      return;
    }
    if(emergencyStopMotor != 0){
      toMotorDriver.emergencyStopMotor = 1;
      toMotorDriver.emergencyStopMotor.write();
      motorState = MotorState::MOTOR_DISABLED;
      return;
    }
  }
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




