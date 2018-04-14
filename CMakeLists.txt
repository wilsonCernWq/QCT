# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required (VERSION 3.5)
project (WarmT)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
include(cmake/packages.cmake)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
add_definitions(-std=c++1z) # cmake 3.5 does not support c17
set(WARMT_BUILD_FLAG   "")
set(WARMT_BUILD_SURFIX "")

# helper library
add_library(utils${WARMT_BUILD_SURFIX}
  utils/image.cpp
  utils/timer.cpp)
target_include_directories(utils${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils;>")
target_link_libraries(utils${WARMT_BUILD_SURFIX} "")
set_target_properties(utils${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG}")

# subdirectories
add_subdirectory(algorithms)

# main program
add_executable(composer${WARMT_BUILD_SURFIX} main.cpp)
target_link_libraries(composer${WARMT_BUILD_SURFIX} 
  utils${WARMT_BUILD_SURFIX} 
  algorithms${WARMT_BUILD_SURFIX}
  MPI)
set_target_properties(composer${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG}")
