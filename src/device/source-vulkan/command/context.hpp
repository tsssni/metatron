#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <metatron/device/command/context.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/print.hpp>
#include <vulkan/vulkan.hpp>
#include <queue>

namespace mtt::command {
    struct Context::Impl final {
        vk::UniqueInstance instance;
        vk::UniqueDevice device;
        vk::UniquePipelineCache pipeline_cache;

        u32 render_family;
        u32 transfer_family;
        vk::Queue queue;
        vk::Queue dma;

        u32 device_memory;
        u32 host_memory;
        bool uma;

        vk::PhysicalDeviceMemoryProperties2 memory_props;
        vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props;

        Impl() noexcept;
        ~Impl() noexcept;

    private:
        auto init_instance() noexcept -> void;
        auto init_device() noexcept -> void;
        auto init_memory() noexcept -> void;
        auto init_pipeline_cache() noexcept -> void;
    };

    auto inline guard(vk::Result result) noexcept -> void {
        if (result != vk::Result::eSuccess) {
            std::println("vulkan error: {}", vk::to_string(result));
            std::abort();
        }
    }

    template<typename T>
    auto guard(rref<vk::ResultValue<T>> result) {
        guard(result.result);
        return std::move(result.value);
    }
}
