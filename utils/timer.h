#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <string>

#ifdef PARALLEL
# include <mpi.h>
#endif

class Timer{
private:
  bool usempi = false;
#ifdef PARALLEL
  double mpi_start_time, mpi_end_time; 
#endif
  std::chrono::time_point<std::chrono::high_resolution_clock> 
    start_time, end_time;
  std::chrono::duration<double> elapsed_seconds;
 public:
  Timer() {
#ifdef PARALLEL
    int mpiSize; MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    usempi = mpiSize > 1;
#endif
  };
  ~Timer() = default;
  void Start();
  void Stop();
  double GetDuration();
  static std::string GetCurrentTime(bool shorter = false); 
};

#endif
