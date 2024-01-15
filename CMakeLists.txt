cmake_minimum_required(VERSION 3.28)
project(tsssni)
set(CMAKE_CXX_STANDARD 20)

define_property(TARGET PROPERTY tsssni-found-libs BRIEF_DOCS "targets found by find_package" FULL_DOCS ".")
define_property(TARGET PROPERTY tsssni-link-libs BRIEF_DOCS "library target needed by a target" FULL_DOCS ".")
define_property(TARGET PROPERTY tsssni-link-srcs BRIEF_DOCS "source target needed by a target" FULL_DOCS ".")

set(TSSSNI_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
set(TSSSNI_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(TSSSNI_EXE_DIR ${CMAKE_CURRENT_LIST_DIR}/exe)
set(TSSSNI_LIB_BUILD_DIR ${CMAKE_BINARY_DIR}/bin/lib)
set(TSSSNI_BIN_BUILD_DIR ${CMAKE_BINARY_DIR}/bin)
set(TSSSNI_GEN_BUILD_DIR ${CMAKE_BINARY_DIR}/gen)

include(lib/setup.cmake)
include(src/setup.cmake)
include(exe/setup.cmake)

