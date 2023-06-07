// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE testMotorServer
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotorServer.h"

#include <ChimeraTK/ApplicationCore/TestFacility.h>

/**********************************************************************************************************************/

// Just checks that the server instance can be instantiated and started
BOOST_AUTO_TEST_CASE(testInstantiaton) {
  StepperMotorServer theServer;
  ChimeraTK::TestFacility testFacility{theServer};
  testFacility.runApplication();
}

/**********************************************************************************************************************/
