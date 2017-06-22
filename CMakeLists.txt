cmake_minimum_required (VERSION 2.8)
project (ImageComposer)

# set flags
add_definitions(-std=c++11)
set(CMAKE_CXX_FLAGS_RELEASE -DNDEBUG)

# find openMP
find_package(OpenMP)

# find MPI
find_package(MPI REQUIRED)
if (MPI_FOUND)
  add_definitions(-DPARALLEL)
endif()

# set include directory
include_directories(
  visit 
  utils
  ${MPI_INCLUDE_PATH}
)

# marco to build project
macro(BUILD_ALL surfix flag)
  message("building ${surfix} version - flags: ${flag}")
  # helper library
  add_library(utils_${surfix}
    visit/avtSLIVRImgCommunicator.C
    visit/avtSLIVRImgMetaData.C
    utils/image.cpp
    utils/timer.cpp
    )
  set_target_properties(utils_${surfix} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS} ${flag}")
  set_target_properties(utils_${surfix} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS} ${flag}")
  # main program
  add_executable(composer_${surfix} main.cpp)
  target_link_libraries(composer_${surfix} ${MPI_LIBRARIES} utils_${surfix})
  set_target_properties(composer_${surfix} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS} ${flag}")
  set_target_properties(composer_${surfix} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS} ${flag}")
endmacro()

#--- reference version
BUILD_ALL("ref" "")
#--- auto optimized version
BUILD_ALL("auto" "-O3")
#--- openMP version
BUILD_ALL("omp" "${OpenMP_CXX_FLAGS}")
#--- full version
BUILD_ALL("full" "-O3 ${OpenMP_CXX_FLAGS}")

