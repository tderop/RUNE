#pragma once

#include <switch.h>
#include <chrono>

// using time_point = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>>;

/**
 * @brief Singleton timer class for timing code execution.
 */
class Timer {
    private:
        static inline std::chrono::steady_clock::time_point start_;
        static inline std::chrono::steady_clock::time_point end_;

    public:
        static void start();
        static void end();

        static uint64_t get();
};