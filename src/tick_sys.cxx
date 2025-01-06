#include "tick_sys.hxx"

void TickSys::start()
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

        std::this_thread::sleep_until(next_tick_time);
    }
}

void TickSys::stop()
{
    running = false;
}