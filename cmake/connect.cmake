function(link unit)
    set(target metatron-${unit})
    get_property(path TARGET ${target} PROPERTY metatron-path)
    get_property(mode TARGET ${target} PROPERTY metatron-mode)
    get_property(access TARGET ${target} PROPERTY metatron-access)

    # execute setup file
    set(metatron-deps)
    set(metatron-ext)
    set(setup-file ${path}/setup.cmake)
    if(EXISTS ${setup-file})
        include(${setup-file})
    endif()

    # solve dependencies
    set(linked-libs)
    foreach(dep ${metatron-deps})
        if(${mode} STREQUAL "ext")
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

    # link core for some global visible utilities
    if(TRUE
    AND NOT ${mode} STREQUAL "lib" 
    AND NOT ${mode} STREQUAL "ext" 
    AND NOT ${unit} STREQUAL "core")
        target_link_libraries(${target} ${access} metatron-core)
    endif()
endfunction()

function(build)
    # compile shaders using metatron-builder
    set(src ${CMAKE_CURRENT_LIST_DIR}/shader)
    set(out ${CMAKE_BINARY_DIR}/shader)

    if(EXISTS ${src})
        message(STATUS "compiling shaders from ${src} to ${out}")
        add_custom_target(
            metatron-shader ALL
            COMMAND metatron-builder -d ${src} -o ${out}
            DEPENDS metatron-builder
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )
    endif()
endfunction()

function(connect)
    get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)
    foreach(unit ${metatron-units})
        link(${unit})
    endforeach()
    build()
endfunction()
