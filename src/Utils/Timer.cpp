#include <Utils/Timer.hpp>

void Timer::start(){
    Timer::start_ = std::chrono::steady_clock::now();
}

void Timer::end(){
    Timer::end_ = std::chrono::steady_clock::now();
}

uint64_t Timer::get(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Timer::end_-Timer::start_).count();
}