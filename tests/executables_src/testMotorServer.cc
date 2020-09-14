// Define a name for the test module.
#define BOOST_TEST_MODULE testMotorServer
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>

#include <ChimeraTK/ApplicationCore/TestFacility.h>

#include "StepperMotorServer.h"


// Declare the server instance
static StepperMotorServer theServer;

static ChimeraTK::TestFacility testFacility;

struct TestFixture {
  TestFixture(){
    testFacility.runApplication();
  }
};
static TestFixture fixture;

using namespace boost::unit_test_framework;


/// TestSuite for the server
BOOST_AUTO_TEST_SUITE(StepperMotorServerTestSuite)

/**********************************************************************************************************************/

/// A template test case
BOOST_AUTO_TEST_CASE(testTemplate){
  std::cout << "testTemplate" << std::endl;
}

/**********************************************************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
