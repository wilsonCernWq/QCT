# ┌──────────────────────────────────────────────────────────────────┐
# │  Common Library                                                  │
# └──────────────────────────────────────────────────────────────────┘
add_library(algoComm${WARMT_BUILD_SURFIX}
  Tile.cpp)
target_include_directories(algoComm${WARMT_BUILD_SURFIX} PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};>")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Different Algorithms                                            │
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(visit)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Create an Overall library                                       │
# └──────────────────────────────────────────────────────────────────┘
add_library(algorithms${WARMT_BUILD_SURFIX}
  Compositor.cpp)
target_include_directories(algorithms${WARMT_BUILD_SURFIX} PRIVATE
  "$<BUILD_INTERFACE:"
  "${CMAKE_CURRENT_SOURCE_DIR};"
  ">")
target_link_libraries(algorithms${WARMT_BUILD_SURFIX} 
  algoComm${WARMT_BUILD_SURFIX}
  algoVisIt${WARMT_BUILD_SURFIX})
set_target_properties(algorithms${WARMT_BUILD_SURFIX} 
  PROPERTIES
  COMPILE_FLAGS "${WARMT_BUILD_FLAG}")
