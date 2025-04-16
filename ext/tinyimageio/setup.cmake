file(GLOB sources ${path}/src/*.cpp)
target_sources(metatron-tinyimageio PRIVATE ${sources})
target_include_directories(
	metatron-tinyimageio PUBLIC 
	$<BUILD_INTERFACE:${path}/src/include>
	$<INSTALL_INTERFACE:include/${target}>
)
