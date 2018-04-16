add_library(algoVisIt${WARMT_BUILD_SURFIX}
  CompositorVisIt.cpp
  avtSLIVRImgCommunicator.C
  avtSLIVRImgMetaData.C)
target_include_directories(algoVisIt${WARMT_BUILD_SURFIX} PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};>")
target_link_libraries(algoVisIt${WARMT_BUILD_SURFIX} 
  algoComm${WARMT_BUILD_SURFIX} 
  icet
  MPI)
set_target_properties(algoVisIt${WARMT_BUILD_SURFIX} 
  PROPERTIES
  COMPILE_FLAGS "${WARMT_BUILD_FLAG}")
