#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE testMotorServer
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <ChimeraTK/ApplicationCore/TestFacility.h>
#include "StepperMotorServer.h"

/**********************************************************************************************************************/

// Just checks that the server instance can be instantiated and started
BOOST_AUTO_TEST_CASE(testInstantiaton) {
  StepperMotorServer theServer;
  ChimeraTK::TestFacility testFacility{theServer};
  testFacility.runApplication();
}

/**********************************************************************************************************************/
