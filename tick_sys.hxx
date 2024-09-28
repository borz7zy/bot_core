#ifndef _TICK_SYS_HXX
#define _TICK_SYS_HXX

#include <thread>
#include <chrono>
#include <functional>

class TickSys
{
public:
    TickSys(uint8_t ticks_per_second,
            std::function<void()> update_function) : ticks_per_second(ticks_per_second),
                                                     tick_duration(1000 / ticks_per_second),
                                                     total_execution_time(0),
                                                     update_function(update_function) {}

    void start();

    void stop();

private:
    uint8_t ticks_per_second;
    int tick_duration;
    bool running = true;
    double total_execution_time;
    std::function<void()> update_function;
};

#endif