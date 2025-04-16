add_executable(metatron-rgb2spec-run ${path}/src/rgb2spec.cpp)
set(rgb2spec-color-spaces sRGB)
set(rgb2spec-src)

foreach(color-space ${rgb2spec-color-spaces})
	set(file ${CMAKE_CURRENT_BINARY_DIR}/rgb2spec/${color-space}-spectrum.cpp)
	list(APPEND rgb2spec-src ${file})
	add_custom_command(
		OUTPUT ${file}
		COMMAND metatron-rgb2spec-run 64 ${file} ${color-space}
		DEPENDS metatron-rgb2spec-run
	)
endforeach()

target_sources(metatron-rgb2spec PUBLIC ${rgb2spec-src})
