/*
 * MotorDriverParameters.h
 *
 *  Created on: Aug 14, 2018
 *      Author: ckampm
 */

#ifndef INCLUDE_MOTORDRIVERPARAMETERS_H_
#define INCLUDE_MOTORDRIVERPARAMETERS_H_

struct MotorDriverParameters {

  std::string const & motorDriverCardDeviceName;
  std::string const & moduleName;
  unsigned int motorDriverId;
  std::string motorDriverCardConfigFileName;
  double encoderUnitToStepsRatio;
};



#endif /* INCLUDE_MOTORDRIVERPARAMETERS_H_ */
