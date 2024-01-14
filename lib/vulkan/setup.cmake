find_package(Vulkan REQUIRED)
set_property(TARGET tsssni.lib.vulkan APPEND PROPERTY tsssni-found-libs Vulkan::Vulkan)
