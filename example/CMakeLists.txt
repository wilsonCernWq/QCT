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
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++17")

# simple program
if (BUILD_EXAMPLE_SIMPLE)
  add_executable(simple${WARMT_BUILD_SURFIX} 
    simple/main.cpp)
  target_link_libraries(simple${WARMT_BUILD_SURFIX} PUBLIC
    utils${WARMT_BUILD_SURFIX}
    qct${WARMT_BUILD_SURFIX}
    MPI)
  target_include_directories(simple${WARMT_BUILD_SURFIX} PUBLIC
    "$<BUILD_INTERFACE:"
    "${CMAKE_CURRENT_SOURCE_DIR}/..;"
    "${CMAKE_CURRENT_SOURCE_DIR}/simple;"
    ">")
  set_target_properties(simple${WARMT_BUILD_SURFIX} 
    PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++17")
endif (BUILD_EXAMPLE_SIMPLE)

# volume viewer program
if (BUILD_EXAMPLE_VOLUME)
  add_executable(volume${WARMT_BUILD_SURFIX} 
    volume/main.cpp)
  target_link_libraries(volume${WARMT_BUILD_SURFIX} PUBLIC
    utils${WARMT_BUILD_SURFIX}
    qct${WARMT_BUILD_SURFIX}
    ${OSPRAY_LIBRARIES}
    glm
    MPI)
  target_include_directories(volume${WARMT_BUILD_SURFIX} PUBLIC
    "$<BUILD_INTERFACE:"
    "${OSPRAY_INCLUDE_DIRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/..;"
    "${CMAKE_CURRENT_SOURCE_DIR}/volume;"
    ">")
  set_target_properties(volume${WARMT_BUILD_SURFIX} 
    PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++17")
endif (BUILD_EXAMPLE_VOLUME)
