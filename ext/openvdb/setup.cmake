modulo(${path})
extend(OpenVDB)
target_include_directories(metatron-openvdb INTERFACE ${OPENVDB_INCLUDE_DIRS})
list(APPEND metatron-deps ${OPENVDB_LIBRARIES})
