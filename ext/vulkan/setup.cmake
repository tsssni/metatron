if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    extend(Vulkan)
    list(APPEND metatron-deps Vulkan::Vulkan Vulkan::Headers)
endif()
