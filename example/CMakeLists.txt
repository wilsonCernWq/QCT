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

# simple program
add_executable(example${WARMT_BUILD_SURFIX} 
  simple/main.cpp)
target_link_libraries(example${WARMT_BUILD_SURFIX} PUBLIC
  utils${WARMT_BUILD_SURFIX}
  qct${WARMT_BUILD_SURFIX}
  MPI)
target_include_directories(example${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:"
  "${CMAKE_CURRENT_SOURCE_DIR}/..;"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils;"
  ">")
set_target_properties(example${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")
