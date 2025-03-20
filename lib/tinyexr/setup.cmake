add_library(tinyexr SHARED ${path}/tinyexr.cpp)
target_include_directories(tinyexr PUBLIC ${path})
target_include_directories(tinyexr PUBLIC ${path}/../stb)
list(APPEND metatron-deps tinyexr stb)
