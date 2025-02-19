cmake_minimum_required(VERSION 3.30.5)

project(metatron-renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

set(metatron-root ${CMAKE_CURRENT_LIST_DIR})
set(metatron-bin ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(metatron-lib ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib)

define_property(TARGET PROPERTY metatron-units)
define_property(TARGET PROPERTY metatron-path)
define_property(TARGET PROPERTY metatron-mode)
define_property(TARGET PROPERTY metatron-access)

include(cmake/classify.cmake)
include(cmake/evaluate.cmake)
include(cmake/link.cmake)

add_library(metatron-build INTERFACE)
set_property(TARGET metatron-build PROPERTY metatron-units)

metatron_classify(${metatron-root}/lib "lib")
metatron_classify(${metatron-root}/src "src")
metatron_classify(${metatron-root}/exe "exe")

get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)
foreach(unit ${metatron-units})
  metatron_link(${unit})
endforeach()
