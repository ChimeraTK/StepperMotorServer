
#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include "MotorDriver.h"


struct TestModule : public ChimeraTK::ApplicationModule{

  using ChimeraTK::ApplicationModule::ApplicationModule;


  ctk::ReadAnyGroup inputGroup;


  //Limit speedLimit{this, "speedLimit", "motor speedLimit"}

  struct Limit : public ctk::VariableGroup {

    using ctk::VariableGroup::VariableGroup;

    ctk::ScalarPushInput<double> userValue{this, "userValue", "", "Speed limit set for the motor"};
    ctk::ScalarPushInput<double> maxValue{this, "maxValue", "", "Maximum velocity of the motor"};

  };
  Limit speedLimit{this, "speedLimit", "", false, {"MOTOR"}};

  ChimeraTK::ScalarOutput<double> userSpeedLimit{this, "SpdLimOut", "", {"CS"}};

  void mainLoop() override{

    inputGroup = speedLimit.readAnyGroup();

    while(true){

      inputGroup.readAny();
      readAll();

      userSpeedLimit = speedLimit.userValue * 2;
      writeAll();
    }
  }

};


