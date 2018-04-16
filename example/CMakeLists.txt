# ┌──────────────────────────────────────────────────────────────────┐
# │  Testing Program                                                 │
# └──────────────────────────────────────────────────────────────────┘
# helper library (this library should not be used outside the project)
add_library(utils${WARMT_BUILD_SURFIX}
  utils/image.cpp
  utils/timer.cpp)
target_include_directories(utils${WARMT_BUILD_SURFIX} PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils;>")
target_link_libraries(utils${WARMT_BUILD_SURFIX} "")
set_target_properties(utils${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")

# testing program
add_executable(example${WARMT_BUILD_SURFIX} main.cpp)
target_link_libraries(example${WARMT_BUILD_SURFIX} PRIVATE
  utils${WARMT_BUILD_SURFIX}
  qct${WARMT_BUILD_SURFIX}
  MPI)
target_include_directories(utils${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;>")
set_target_properties(example${WARMT_BUILD_SURFIX} 
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")
