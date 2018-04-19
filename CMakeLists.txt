# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required (VERSION 3.5)
project(QCT VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Options                                                         │
# └──────────────────────────────────────────────────────────────────┘
option(BUILD_EXAMPLE_SIMPLE "Build simple example" ON)
option(BUILD_EXAMPLE_VOLUME "Build volume rendering example" OFF)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
if (BUILD_EXAMPLE_VOLUME)
  set(IMPORT_GLM ON)
  set(IMPORT_OSPRAY ON)
endif ()
include(cmake/packages.cmake)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
set(WARMT_BUILD_FLAG   "")
set(WARMT_BUILD_SURFIX "")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Add Sub-Directories                                             │
# └──────────────────────────────────────────────────────────────────┘ 
add_subdirectory(algorithms)

# ┌──────────────────────────────────────────────────────────────────┐
# │  QCT Library                                                     │
# └──────────────────────────────────────────────────────────────────┘
add_library(qct${WARMT_BUILD_SURFIX} qct/api.cpp)
target_include_directories(qct${WARMT_BUILD_SURFIX} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};>")
target_link_libraries(qct${WARMT_BUILD_SURFIX} PRIVATE
  algorithms${WARMT_BUILD_SURFIX})
set_target_properties(qct${WARMT_BUILD_SURFIX}
  PROPERTIES COMPILE_FLAGS "${WARMT_BUILD_FLAG} -std=c++1z")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Example Program                                                 │
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(example)
