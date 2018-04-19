add_library(algoTree${WARMT_BUILD_SURFIX}
  CompositorTree.cpp)
target_include_directories(algoTree${WARMT_BUILD_SURFIX} PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};>")
target_link_libraries(algoTree${WARMT_BUILD_SURFIX} 
  algoComm${WARMT_BUILD_SURFIX}
  MPI)
set_target_properties(algoTree${WARMT_BUILD_SURFIX} 
  PROPERTIES
  COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++17")
