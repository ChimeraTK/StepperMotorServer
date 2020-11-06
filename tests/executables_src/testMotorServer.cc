// Define a name for the test module.
#define BOOST_TEST_MODULE testMotorServer
// Only after defining the name include the unit test header.
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <ChimeraTK/ApplicationCore/TestFacility.h>
#include "StepperMotorServer.h"

/**********************************************************************************************************************/

// Just checks that the server instance can be instantiated and started
BOOST_AUTO_TEST_CASE(testInstantiaton) {
  StepperMotorServer theServer;
  ChimeraTK::TestFacility testFacility;
  testFacility.runApplication();
}

/**********************************************************************************************************************/
