file(GLOB sources ${path}/src/*.cpp)
target_sources(metatron-tevipc PRIVATE ${sources})
target_include_directories(
    metatron-tevipc PUBLIC 
    $<BUILD_INTERFACE:${path}/src/include>
    $<INSTALL_INTERFACE:include/${target}>
)
