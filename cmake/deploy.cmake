function(derive unit)
    set(target metatron-${unit})
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

    if(${mode} STREQUAL "src" AND EXISTS ${path}/include)
        install(
            DIRECTORY ${path}/include/
            DESTINATION include/${target}
        )
    elseif(${mode} STREQUAL "lib" AND EXISTS ${path}/src/include)
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
        DESTINATION share/
    )
endfunction()

function(deploy)
    get_property(metatron-units TARGET metatron-build PROPERTY metatron-units)

    foreach(unit ${metatron-units})
        derive(${unit})
    endforeach()

    release()
endfunction()
