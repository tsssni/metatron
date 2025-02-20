function(metatron_link unit)
	set(target metatron-${unit})
	get_property(path TARGET ${target} PROPERTY metatron-path)
	get_property(mode TARGET ${target} PROPERTY metatron-mode)
	get_property(access TARGET ${target} PROPERTY metatron-access)

	# execute setup file
	set(metatron-deps)
	set(setup-file ${path}/setup.cmake)
	if(EXISTS ${setup-file})
		include(${setup-file})
	endif()

	# solve dependencies
	set(linked-libs)
	foreach(dep ${metatron-deps})
		if(${mode} STREQUAL "lib")
			list(APPEND linked-libs ${dep})
		else()
			list(APPEND linked-libs metatron-${dep})
		endif()
	endforeach()

	# link dependencies
	if(linked-libs)
		message(STATUS "${target} link libraries ${linked-libs}")
		target_link_libraries(${target} ${access} ${linked-libs})
	endif()

	# add prelude for some global visible utilities
	if(TRUE
	AND NOT ${mode} STREQUAL "lib" 
	AND NOT ${unit} STREQUAL "prelude")
		target_link_libraries(${target} ${access} metatron-prelude)
	endif()
endfunction()
