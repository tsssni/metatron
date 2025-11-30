#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <vulkan/vulkan.hpp>

namespace mtt::command {
    struct Context final: stl::singleton<Context> {
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkCommandPool command_pool;
        Context() noexcept;
    };
}
