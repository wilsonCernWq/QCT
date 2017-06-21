#include <chrono>
#include <string>
#include <ctime>
#include <sstream> // stringstream

#ifndef _TIMER_H_
#define _TIMER_H_

class Timer{
private:
    std::chrono::time_point<std::chrono::system_clock> start_time, end_time;
    std::chrono::duration<double> elapsed_seconds;
public:
    Timer() = default;
    ~Timer() = default;
    void Start();
    void Stop();
    double GetDuration();
    static std::string GetCurrentTime(bool shorter = false); 
};

#endif
