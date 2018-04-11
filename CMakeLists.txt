# setup cmake
cmake_minimum_required (VERSION 3.5)
project (HotT)
add_definitions(-std=c++1z) # cmake 3.5 does not support c17
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

# find TBB 
find_package(TBB QUIET)

# find MPI
find_package(MPI REQUIRED)
if (MPI_FOUND)

  # message(STATUS "MPI_CXX_INCLUDE_PATH: ${MPI_CXX_INCLUDE_PATH}")
  # message(STATUS "MPI_CXX_COMPILE_FLAGS: ${MPI_CXX_COMPILE_FLAGS}")
  # message(STATUS "MPI_CXX_LINK_FLAGS: ${MPI_CXX_LINK_FLAGS}")
  # message(STATUS "MPI_CXX_LIBRARIES: ${MPI_CXX_LIBRARIES}")
  
  add_library(MPI INTERFACE IMPORTED)
  set_target_properties(MPI PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS PARALLEL=1
    INTERFACE_INCLUDE_DIRECTORIES "${MPI_CXX_INCLUDE_PATH}"
    INTERFACE_LINK_LIBRARIES "${MPI_CXX_LIBRARIES}")

  if (MPI_CXX_COMPILE_FLAGS)
    set_target_properties(MPI PROPERTIES 
      INTERFACE_COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
  endif (MPI_CXX_COMPILE_FLAGS)
  if (MPI_CXX_LINK_FLAGS)
    set_target_properties(MPI PROPERTIES 
      INTERFACE_LINK_FLAGS "${MPI_CXX_LINK_FLAGS}")
  endif (MPI_CXX_LINK_FLAGS)

endif()

# marco to build project
macro(BUILD_ALL surfix flag)

  message("building ${surfix} version - flags: ${flag}")

  # helper library
  add_library(utils_${surfix}
    algorithms/visit/avtSLIVRImgCommunicator.C
    algorithms/visit/avtSLIVRImgMetaData.C
    utils/image.cpp
    utils/timer.cpp)
  target_include_directories(utils_${surfix} PUBLIC
    "$<BUILD_INTERFACE:"
    "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/visit;"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils>")
  target_link_libraries(utils_${surfix} 
    MPI)
  set_target_properties(utils_${surfix} 
    PROPERTIES
    COMPILE_FLAGS "${flag}")

  # main program
  add_executable(composer_${surfix} main.cpp)
  target_link_libraries(composer_${surfix} 
    utils_${surfix}
    MPI)
  set_target_properties(composer_${surfix} 
    PROPERTIES
    COMPILE_FLAGS "${flag}")

endmacro()

BUILD_ALL("o3" "-O3")
