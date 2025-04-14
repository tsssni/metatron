cmake_minimum_required(VERSION 3.30.5)

project(metatron-renderer)
set(CMAKE_CXX_STANDARD 23)

define_property(TARGET PROPERTY metatron-units)
define_property(TARGET PROPERTY metatron-path)
define_property(TARGET PROPERTY metatron-mode)
define_property(TARGET PROPERTY metatron-access)

include(cmake/classify.cmake)
include(cmake/evaluate.cmake)
include(cmake/link.cmake)

add_library(metatron-build INTERFACE)
set_property(TARGET metatron-build PROPERTY metatron-units)

metatron_classify(${CMAKE_CURRENT_LIST_DIR}/ext "ext")
metatron_classify(${CMAKE_CURRENT_LIST_DIR}/src "src")
metatron_classify(${CMAKE_CURRENT_LIST_DIR}/exe "exe")

get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)
foreach(unit ${metatron-units})
	metatron_link(${unit})
endforeach()
