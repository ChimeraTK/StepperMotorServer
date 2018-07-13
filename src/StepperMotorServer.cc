/*
 * StepperMotorServer.cc
 *
 *  Created on: Jun 8, 2018
 *      Author: ckampm
 */

#include <iostream>
#include <string>

#include "StepperMotorServer.h"
#include "version.h"

// FIXME
//static int __TEMP_CS_INPUT_TBD_ = 0 ;

void StepperMotorServer::defineConnections(){

  std::string dMapFileName{"dummies.dmap"};

  ChimeraTK::setDMapFilePath(dMapFileName);

  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor server version "
            << AppVersion::major << "." << AppVersion::minor << "." << AppVersion::patch << std::endl;


  //TODO Current wiring of the trigger and other push-type inputs
  //     leads to updates inbetween samples. Rearrange triggers to have a
  //     'hard' cycle time

  // Setup poll trigger
  //auto cycleTime = config.get<int32_t>("cycleTime");
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs("TIMER.UPDATE.ONCE") >> trigger.forceUpdate;
  cs("TIMER.UPDATE.AUTO") >> trigger.automaticUpdate;
  trigger.countdown >> cs("TIMER.COUNTDOWN");

  auto &triggerNr = trigger.trigger;

  // Connect motorControl signals
  motorControl.findTag("CS").connectTo(cs, triggerNr);



  /*
   * Control system interface
   * TODO This is for now defined as a spec taken from the BAM motor server
   */
   // First define signals in application modules, then map

} /* defineConnections() */
