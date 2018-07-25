
#ifndef INCLUDE_TRIGGER_H_
#define INCLUDE_TRIGGER_H_

#include <ChimeraTK/ApplicationCore/Application.h>
#include <ChimeraTK/ReadAnyGroup.h>
#include <ChimeraTK/ApplicationCore/ApplicationModule.h>

#include <chrono>

namespace ctk = ChimeraTK;

// TODO Evaluate this approach. Does it work  also with testing? Eventually move to App'Core


struct Trigger : public ctk::ApplicationModule {
    using ApplicationModule::ApplicationModule;
    virtual ~Trigger() {}

    ctk::ReadAnyGroup inputGroup;

    ctk::ScalarOutput<int> trigger{this, "trigger", "", ""};
    ctk::ScalarOutput<int> countdown{this, "countdown", "", ""};
    ctk::ScalarPushInput<int> forceUpdate{this, "forceUpdate", "", "Force an update of the ulog values"};
    ctk::ScalarPushInput<int> automaticUpdate{this, "automaticUpdate", "", "Whether the values should should be polled periodically from the device"};
    ctk::ScalarPushInput<int> tick{this, "timerTick", "", "Tick of the 1Hz timer"};

    void mainLoop()
    {
        trigger = 0;
        countdown = 0;

        inputGroup = readAnyGroup();

        while (true)
        {
            auto id = inputGroup.readAny();
            if (id == forceUpdate.getId() && automaticUpdate == 0) {
                trigger++;
                trigger.write();
            } else if (id == automaticUpdate.getId()) {
                // This will schedule one last update before going into manual mode and also refresh
                // the current values right after enabling auto-update again
                countdown = 0;
                trigger++;
                writeAll();
            } else if (id == tick.getId() && automaticUpdate != 0) {
                // FIXME Countdown is the same as tick, if we set up the timer using sleep_for(period)
                countdown = static_cast<int>(tick);
                // Timer has spoken, trigger update if we have reached timeout (tick == 0) and
                // the user wants us to update
//                if (tick == 0) {
//                    trigger++;
//                    trigger.write();
//                }
                trigger++;
                trigger.write();
                countdown.write();
            }
        }
    }
};
template <typename timebaseT = std::chrono::milliseconds>
struct Timer : public ctk::ApplicationModule {
    using ApplicationModule::ApplicationModule;

    // TODO: template description?
    ctk::ScalarPollInput<int32_t> timeout{this, "timeout", "", "Timeout (user-defined unit)"};
    ctk::ScalarOutput<int> tick{this, "tick", "", "Timer tick"};

    void mainLoop()
    {
        tick = 0;
        while (true) {
//            std::this_thread::sleep_for(timebaseT(1));
//            tick++;
//            tick %= timeout;
//            tick.write();
          tick++;
          tick.write();
          int period = timeout;
          std::this_thread::sleep_for(timebaseT(period));
        }
    }
};


#endif /* INCLUDE_TRIGGER_H_ */
