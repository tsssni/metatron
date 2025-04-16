target_sources(metatron-tinyexr PUBLIC ${path}/src/tinyexr.cpp)
target_include_directories(metatron-tinyexr PUBLIC ${path}/src/include)
target_link_libraries(metatron-tinyexr PUBLIC metatron-stb)
