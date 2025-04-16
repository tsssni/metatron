function(prepare)
	define_property(TARGET PROPERTY metatron-units)
	define_property(TARGET PROPERTY metatron-exts)
	define_property(TARGET PROPERTY metatron-mods)

	define_property(TARGET PROPERTY metatron-path)
	define_property(TARGET PROPERTY metatron-mode)
	define_property(TARGET PROPERTY metatron-access)

	add_library(metatron-build INTERFACE)
	set_property(TARGET metatron-build PROPERTY metatron-units)
endfunction()
