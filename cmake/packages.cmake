# ┌──────────────────────────────────────────────────────────────────┐
# │  Find All the Packages                                           │
# └──────────────────────────────────────────────────────────────────┘

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

# find IceT
find_package(IceT)
if (IceT_FOUND)
  add_library(icet INTERFACE IMPORTED)
  set(ICET_LIBS "")
  foreach(l ${ICET_CORE_LIBS} ${ICET_MPI_LIBS})
    list(APPEND ICET_LIBS ${IceT_DIR}/lib${l}.a)
  endforeach()
  set_target_properties(icet PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS VISIT_ICET=1
    INTERFACE_INCLUDE_DIRECTORIES "${ICET_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${ICET_LIBS}")
endif (IceT_FOUND)
