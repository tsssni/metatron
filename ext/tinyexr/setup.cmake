add_library(tinyexr SHARED ${path}/tinyexr.cpp)
target_include_directories(tinyexr PUBLIC ${path})
target_link_libraries(tinyexr PUBLIC stb)
list(APPEND metatron-deps tinyexr)
