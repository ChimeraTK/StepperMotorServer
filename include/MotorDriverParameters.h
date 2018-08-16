/*
 * MotorDriverParameters.h
 *
 *  Created on: Aug 14, 2018
 *      Author: kampmeyer
 */

#ifndef INCLUDE_MOTORDRIVERPARAMETERS_H_
#define INCLUDE_MOTORDRIVERPARAMETERS_H_

struct MotorDriverParameters {
  std::string const & motorDriverCardDeviceName;
  std::string const & moduleName;
  unsigned int motorDriverId;
  std::string motorDriverCardConfigFileName;
};



#endif /* INCLUDE_MOTORDRIVERPARAMETERS_H_ */
