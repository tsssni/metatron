function(derive unit)
    set(target metatron-${unit})
    if(NOT TARGET ${target})
        return()
    endif()
    get_property(path TARGET ${target} PROPERTY metatron-path)
    get_property(mode TARGET ${target} PROPERTY metatron-mode)

    if(EXISTS ${path}/include)
        install(
            TARGETS ${target}
            EXPORT metatron-targets
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            INCLUDES DESTINATION include/${target}
        )
    else()
        install(
            TARGETS ${target}
            EXPORT metatron-targets
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
        )
    endif()

    if(${mode} STREQUAL "lib" AND EXISTS ${path}/src/include)
        install(
            DIRECTORY ${path}/src/include/
            DESTINATION include/${target}
        )
    endif()
endfunction()

function(release)
    get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)
    get_property(metatron-exts TARGET metatron-build PROPERTY metatron-exts)
    get_property(metatron-mods TARGET metatron-build PROPERTY metatron-mods)

    install(
        TARGETS metatron
        EXPORT metatron-targets
        LIBRARY DESTINATION lib
    )

    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/include/metatron/
        DESTINATION include/metatron
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Inject the prelude include at the top of every installed header so
    # downstream code that uses prelude aliases (cref<T>, f32, ...) compiles
    # without any extra compile flag. The prelude itself is skipped.
    install(CODE [[
        file(GLOB_RECURSE _hpps "${CMAKE_INSTALL_PREFIX}/include/metatron/*.hpp")
        set(_prelude "${CMAKE_INSTALL_PREFIX}/include/metatron/core/prelude/prelude.hpp")
        foreach(_h ${_hpps})
            if(NOT _h STREQUAL _prelude)
                file(READ "${_h}" _content)
                if(NOT _content MATCHES "metatron/core/prelude/prelude\\.hpp")
                    file(WRITE "${_h}" "#include <metatron/core/prelude/prelude.hpp>\n${_content}")
                endif()
            endif()
        endforeach()
    ]])

    install(
        EXPORT metatron-targets
        FILE metatron-targets.cmake
        DESTINATION lib/cmake/metatron
    )
    install(
        FILES ${metatron-mods}
        DESTINATION lib/cmake/metatron
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        cmake/metatron-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/metatron-config.cmake
        INSTALL_DESTINATION lib/cmake/metatron
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/metatron-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/metatron-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/metatron-version.cmake
        DESTINATION lib/cmake/metatron
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/share/
        DESTINATION share
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shader/
        DESTINATION share/metatron/shader
    )
endfunction()

function(deploy)
    get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)

    foreach(unit ${metatron-units})
        derive(${unit})
    endforeach()

    release()
endfunction()
