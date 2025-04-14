list(APPEND CMAKE_MODULE_PATH ${path})
find_package(OpenVDB REQUIRED)
target_include_directories(metatron-openvdb INTERFACE ${OPENVDB_INCLUDE_DIRS})
list(APPEND metatron-deps ${OPENVDB_LIBRARIES})
