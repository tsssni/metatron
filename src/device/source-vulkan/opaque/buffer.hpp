#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::opaque {
    struct Barrier final {
        vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eBottomOfPipe;
        vk::AccessFlags2 access = vk::AccessFlagBits2::eNone;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        u32 family = math::maxv<u32>;

        template<typename T>
        auto update(cref<Barrier> desc) noexcept -> T {
            auto transfer = desc.family != math::maxv<u32>;
            auto barrier = T{
                .srcStageMask = stage,
                .srcAccessMask = access,
                .dstStageMask = desc.stage,
                .dstAccessMask = desc.access,
                .srcQueueFamilyIndex = family,
                .dstQueueFamilyIndex = transfer ? desc.family : family,
            };
            if constexpr (std::is_same_v<T, vk::ImageMemoryBarrier2>) {
                barrier.oldLayout = layout;
                barrier.newLayout = desc.layout;
            }
            stage = desc.stage;
            access = desc.access;
            layout = desc.layout;
            family = transfer ? desc.family : family;
            return barrier;
        }
    };

    struct Buffer::Impl final {
        vk::MemoryAllocateFlags static flags;
        vk::BufferUsageFlags2 static usages;

        Barrier barrier;
        vk::UniqueBuffer device_buffer;
        vk::UniqueBuffer host_buffer;
        vk::UniqueDeviceMemory device_memory;
        vk::UniqueDeviceMemory host_memory;

        auto update(cref<Barrier> desc) noexcept -> vk::BufferMemoryBarrier2;
    };
}
