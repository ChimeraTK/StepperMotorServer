/*
 * testMotorDriver.cc
 *
 *  Created on: Jul 18, 2018
 *      Author: ckampm
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverTest
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotorModule.h"
#include "ChimeraTK/ApplicationCore/TestFacility.h"
//#include "mtca4u/MotorDriverCard/StepperMotor.h"

#include "ChimeraTK/Device.h"

#include <iostream>

namespace ctk = ChimeraTK;

static ChimeraTK::TestFacility testFacility;

static const std::string stepperMotorDeviceName("MOTOR_DUMMY");
static const std::string stepperMotorDeviceConfigFile("motorConfig.xml");
static const std::string dmapPath(".");
static const std::string stepperMotorModuleName("MD22.1");


void setup(){
  testFacility.writeScalar("Motor1/controlInput/enable", 0);
}


BOOST_AUTO_TEST_SUITE( MotorDriverTestSuite )

BOOST_AUTO_TEST_CASE( testOpenMotor ) {

}

/*********************************************************************************************************************/
BOOST_AUTO_TEST_SUITE_END()
