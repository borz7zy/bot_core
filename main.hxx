#ifndef _MAIN_HXX
#define _MAIN_HXX

#include "config_manager.hxx"
#include "core.hxx"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>

ConfigManager cManager;
ConfigManager *config = &cManager;

void processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output)
{
    std::istringstream iss(scripts);
    std::string script;
    while (iss >> script)
    {
        output.push_back(basePath + "/" + script);
    }
}

class TickSys
{
public:
    TickSys(int ticks_per_second,
            std::function<void()> update_function)
        : ticks_per_second(ticks_per_second),
          tick_duration(1000 / ticks_per_second),
          tick_count(0),
          total_execution_time(0),
          update_function(update_function) {}

    void start()
    {
        using clock = std::chrono::high_resolution_clock;
        auto next_tick_time = clock::now();

        while (running)
        {
            auto tick_start_time = clock::now();

            next_tick_time += std::chrono::milliseconds(tick_duration);

            update_function();

            auto tick_end_time = clock::now();
            std::chrono::duration<double, std::milli> tick_execution_time = tick_end_time - tick_start_time;
            total_execution_time += tick_execution_time.count();
            ++tick_count;

            std::this_thread::sleep_until(next_tick_time);
        }
    }

    void stop()
    {
        running = false;
    }

private:
    int ticks_per_second;
    int tick_duration;
    bool running = true;
    uint64_t tick_count;
    double total_execution_time;
    std::function<void()> update_function;
};

#endif