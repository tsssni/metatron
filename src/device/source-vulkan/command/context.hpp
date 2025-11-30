#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/print.hpp>
#include <vulkan/vulkan.hpp>

namespace mtt::command {
    struct Context final: stl::singleton<Context>, stl::capsule<Context> {
        vk::Device device;
        u32 render_queue_family; u32 render_queue_size;
        u32 copy_queue_family; u32 copy_queue_size;
        struct Impl;
        Context() noexcept;
    };

    template<typename T>
    auto guard(rref<vk::ResultValue<T>> result) {
        if (result.result != vk::Result::eSuccess) {
            std::println("vulkan error: {}", vk::to_string(result.result));
            std::abort();
        }
        return std::move(result.value);
    }
}
