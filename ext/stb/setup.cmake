file(GLOB stb_sources ${path}/stb_*.cpp)
add_library(stb SHARED ${stb_sources})
target_include_directories(stb PUBLIC ${path})
list(APPEND metatron-deps tinyexr)
