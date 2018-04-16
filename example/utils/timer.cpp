#include "timer.h"
#include <ctime>
#include <sstream> // stringstream

///////////////////////////////////////////////////////////////////////////////
// implementation
///////////////////////////////////////////////////////////////////////////////

void Timer::Start()
{
#ifdef PARALLEL
  if (usempi) { mpi_start_time = MPI_Wtime(); } else
#endif
  {
    start_time = std::chrono::system_clock::now(); 
  }
}

void Timer::Stop() 
{ 
#ifdef PARALLEL
  if (usempi) { mpi_end_time = MPI_Wtime(); } else
#endif
  {
    end_time = std::chrono::system_clock::now(); 
    elapsed_seconds = end_time - start_time; 
  }
}

double Timer::GetDuration() {
#ifdef PARALLEL
  if (usempi) { return mpi_end_time - mpi_start_time; } else
#endif
  {
    return elapsed_seconds.count(); 
  }
}	

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
