add_executable(rgb2spec-run ${path}/rgb2spec.cpp)
set(rgb2spec-color-spaces sRGB)
set(rgb2spec-src)

foreach(color-space ${rgb2spec-color-spaces})
	set(file ${CMAKE_CURRENT_BINARY_DIR}/rgb2spec/${color-space}-spectrum.cpp)
	list(APPEND rgb2spec-src ${file})
	add_custom_command(
		OUTPUT ${file}
		COMMAND rgb2spec-run 64 ${file} ${color-space}
		DEPENDS rgb2spec-run
	)
endforeach()

add_library(rgb2spec SHARED ${rgb2spec-src})
list(APPEND metatron-deps rgb2spec)

