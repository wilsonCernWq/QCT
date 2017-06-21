cmake_minimum_required (VERSION 2.6)
project (ImageComposer)

set(CMAKE_C_COMPILER /opt/intel/compilers_and_libraries_2017.2.174/linux/bin/intel64/icc)
set(CMAKE_CXX_COMPILER /opt/intel/compilers_and_libraries_2017.2.174/linux/bin/intel64/icc)

# enable c11
add_definitions(-std=c++11)
add_definitions(-O3)

# find openMP
find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# find MPI
find_package(MPI REQUIRED)

# set include directory
include_directories(
  visit 
  utils
  ${MPI_INCLUDE_PATH}
)

# helper library
add_library(utils 
  visit/avtSLIVRImgCommunicator.C
  visit/avtSLIVRImgMetaData.C
  utils/image.cpp
  utils/timer.cpp
)

# main program
add_executable(composer main.cpp)
target_link_libraries(composer ${MPI_LIBRARIES} utils)
if (MPI_FOUND)
  add_definitions(-DPARALLEL)
  target_link_libraries(composer ${MPI_LIBRARIES})
  if(MPI_COMPILE_FLAGS)
    set_target_properties(composer PROPERTIES
      COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
  endif()
  if(MPI_LINK_FLAGS)
    set_target_properties(composer PROPERTIES
      LINK_FLAGS "${MPI_LINK_FLAGS}")
  endif()
endif()
