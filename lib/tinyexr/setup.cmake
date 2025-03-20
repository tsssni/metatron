add_library(tinyexr SHARED ${path}/tinyexr.cpp)
target_include_directories(tinyexr PUBLIC ${path})
list(APPEND metatron-deps tinyexr stb)
