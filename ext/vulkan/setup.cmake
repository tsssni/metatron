if(NOT system MATCHES "darwin")
    extend(Vulkan)
    list(APPEND deps Vulkan::Vulkan Vulkan::Headers)
endif()
