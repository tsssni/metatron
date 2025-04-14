find_path(OPENVDB_INCLUDE_DIR
  NAMES openvdb/openvdb.h
  PATH_SUFFIXES include
)

find_library(OPENVDB_LIBRARY
  NAMES openvdb
  PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVDB DEFAULT_MSG 
	OPENVDB_LIBRARY
	OPENVDB_INCLUDE_DIR
)

if(OPENVDB_FOUND)
  set(OPENVDB_LIBRARIES ${OPENVDB_LIBRARY})
  set(OPENVDB_INCLUDE_DIRS ${OPENVDB_INCLUDE_DIR})
endif()
