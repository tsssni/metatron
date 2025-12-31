find_path(NANOVDB_INCLUDE_DIR
    NAMES nanovdb/NanoVDB.h
    PATH_SUFFIXES include
    REQUIRED
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NanoVDB DEFAULT_MSG
    NANOVDB_INCLUDE_DIR
)

add_library(NanoVDB::nanovdb INTERFACE IMPORTED)
target_include_directories(NanoVDB::nanovdb INTERFACE "${NANOVDB_INCLUDE_DIR}")
find_package(ZLIB REQUIRED)
target_compile_definitions(NanoVDB::nanovdb INTERFACE NANOVDB_USE_ZIP)
target_link_libraries(NanoVDB::nanovdb INTERFACE ZLIB::ZLIB)
