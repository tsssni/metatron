function(prepare)
    define_property(TARGET PROPERTY metatron-units)
    define_property(TARGET PROPERTY metatron-exts)
    define_property(TARGET PROPERTY metatron-mods)

    define_property(TARGET PROPERTY metatron-path)
    define_property(TARGET PROPERTY metatron-mode)
    define_property(TARGET PROPERTY metatron-access)

    add_library(metatron-build INTERFACE)
    add_compile_definitions(MTT_PREFIX="${CMAKE_INSTALL_PREFIX}")
    set_property(TARGET metatron-build PROPERTY metatron-units)
endfunction()
