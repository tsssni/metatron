function(link unit)
    set(target metatron-${unit})
    get_property(deps TARGET ${target} PROPERTY metatron-deps)
    get_property(path TARGET ${target} PROPERTY metatron-path)
    get_property(mode TARGET ${target} PROPERTY metatron-mode)
    get_property(access TARGET ${target} PROPERTY metatron-access)

    set(libs)
    foreach(dep ${deps})
        if(${mode} STREQUAL "ext")
            list(APPEND libs ${dep})
        else()
            list(APPEND libs metatron-${dep})
        endif()
    endforeach()

    if(libs)
        message(STATUS "${target} link libraries ${libs}")
        target_link_libraries(${target} ${access} ${libs})
    endif()
endfunction()

function(build)
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
