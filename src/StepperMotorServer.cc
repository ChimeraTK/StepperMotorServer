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

void StepperMotorServer::defineConnections(){

  std::string dMapFileName{"dummies.dmap"};

  ChimeraTK::setDMapFilePath(dMapFileName);

  std::cout << "****************************************************************" << std::endl;
  std::cout << "*** ChimeraTK Stepper Motor server version "
            << AppVersion::major << "." << AppVersion::minor << "." << AppVersion::patch << std::endl;


  // Setup poll trigger
  //auto cycleTime = config.get<int32_t>("cycleTime");
  config("cycleTime") >> timer.timeout;
  timer.tick >> trigger.tick;
  cs("TIMER.UPDATE.ONCE") >> trigger.forceUpdate;
  cs("TIMER.UPDATE.AUTO") >> trigger.automaticUpdate;
  trigger.countdown >> cs("TIMER.COUNTDOWN");

  // Connect motorControl signals
  motorControl.findTag("CS").connectTo(cs);

} /* defineConnections() */




