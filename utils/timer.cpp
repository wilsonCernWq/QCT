#include "timer.h"

///////////////////////////////////////////////////////////////////////////////
// implementation
///////////////////////////////////////////////////////////////////////////////

void Timer::Start()
{ start_time = std::chrono::system_clock::now(); }

void Timer::Stop() 
{ 
    end_time = std::chrono::system_clock::now(); 
    elapsed_seconds = end_time - start_time; 
}

double Timer::GetDuration(){ return elapsed_seconds.count(); }	

std::string Timer::GetCurrentTime(bool shorter)
{
    // get time
    time_t now = time(0);
    tm *local_time = localtime(&now);
    // create string
    std::stringstream ss;
    if (shorter) {
	ss << 1 + local_time->tm_mon 
	   << local_time->tm_mday 
	   << local_time->tm_hour 
	   << local_time->tm_min 
	   << local_time->tm_sec 
	   << std::endl;
    }
    else {
	ss << 1 + local_time->tm_mon 
	   << "-" << local_time->tm_mday 
	   << "-" << local_time->tm_hour 
	   << "-" << local_time->tm_min 
	   << "-" << local_time->tm_sec 
	   << std::endl;
    }
    return ss.str();
}
