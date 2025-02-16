cmake_minimum_required(VERSION 3.30.5)

project(metatron-renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

set(metatron-root ${CMAKE_CURRENT_LIST_DIR})
set(metatron-src ${metatron-root}/src)
set(metatron-bin ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(metatron-lib ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(metatron-gen ${CMAKE_BINARY_DIR}/gen)

include(cmake/evaluate.cmake)
include(lib/setup.cmake)
include(src/setup.cmake)
include(exe/setup.cmake)

