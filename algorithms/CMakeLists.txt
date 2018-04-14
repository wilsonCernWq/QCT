add_library(algorithms${WARMT_BUILD_SURFIX}
  # parent class
  Compositor.cpp
  # method: VisIt
  visit/CompositorVisIt.cpp
  visit/avtSLIVRImgCommunicator.C
  visit/avtSLIVRImgMetaData.C
  # method: tree
  )
target_include_directories(algorithms${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:"
  "${CMAKE_CURRENT_SOURCE_DIR};"
  "${CMAKE_CURRENT_SOURCE_DIR}/tree;"
  "${CMAKE_CURRENT_SOURCE_DIR}/visit;"
  ">")
target_link_libraries(algorithms${WARMT_BUILD_SURFIX} 
  utils${WARMT_BUILD_SURFIX} 
  icet
  MPI)
set_target_properties(algorithms${WARMT_BUILD_SURFIX} 
  PROPERTIES
  COMPILE_FLAGS "${WARMT_BUILD_FLAG}")
