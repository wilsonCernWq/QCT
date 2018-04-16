# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required (VERSION 3.5)
project(QCT VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
include(cmake/packages.cmake)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
set(WARMT_BUILD_FLAG   "")
set(WARMT_BUILD_SURFIX "")

# subdirectories
add_subdirectory(algorithms)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Testing Program                                                 │
# └──────────────────────────────────────────────────────────────────┘
# helper library (this library should not be used outside the project)
add_library(utils${WARMT_BUILD_SURFIX}
  utils/image.cpp
  utils/timer.cpp)
target_include_directories(utils${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils;>")
target_link_libraries(utils${WARMT_BUILD_SURFIX} "")
set_target_properties(utils${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")

# testing program
add_executable(example${WARMT_BUILD_SURFIX} main.cpp)
target_link_libraries(example${WARMT_BUILD_SURFIX} 
  utils${WARMT_BUILD_SURFIX} 
  algorithms${WARMT_BUILD_SURFIX}
  MPI)
set_target_properties(example${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")
