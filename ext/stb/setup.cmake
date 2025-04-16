file(GLOB stb_sources ${path}/src/stb_*.cpp)
target_sources(metatron-stb PUBLIC ${stb_sources})
target_include_directories(metatron-stb PUBLIC ${path}/src/include)
