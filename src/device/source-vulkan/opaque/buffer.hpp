#pragma once
#include "../command/context.hpp"
#include "../command/queue.hpp"
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::opaque {
    struct Barrier final {
        vk::PipelineStageFlags2 stage = vk::PipelineStageFlagBits2::eBottomOfPipe;
        vk::AccessFlags2 access = vk::AccessFlagBits2::eNone;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        u32 family = math::maxv<u32>;

        auto operator==(cref<Barrier> barrier) const noexcept -> bool {
            return true
            && stage == barrier.stage
            && access == barrier.access
            && layout == barrier.layout
            && family == barrier.family;
        }

        template<typename T>
        auto update(cref<Barrier> desc) noexcept -> T {
            auto barrier = T{
                .srcStageMask = stage,
                .srcAccessMask = access,
                .dstStageMask = desc.stage,
                .dstAccessMask = desc.access,
                .srcQueueFamilyIndex = family,
                .dstQueueFamilyIndex = family,
            };
            if constexpr (std::is_same_v<T, vk::ImageMemoryBarrier2>) {
                barrier.oldLayout = layout;
                barrier.newLayout = desc.layout;
            }
            stage = desc.stage;
            access = desc.access;
            layout = desc.layout;
            return barrier;
        }

        template<typename T>
        auto update(mut<command::Queue> dst, mut<command::Queue> src) noexcept -> T {
            auto barrier = T{
                .srcStageMask = stage,
                .srcAccessMask = access,
                .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                .dstAccessMask = vk::AccessFlagBits2::eNone,
                .srcQueueFamilyIndex = command::Queue::Impl::families[u32(src->type)].idx,
                .dstQueueFamilyIndex = command::Queue::Impl::families[u32(dst->type)].idx,
            };
            if constexpr (std::is_same_v<T, vk::ImageMemoryBarrier2>) {
                barrier.oldLayout = layout;
                barrier.newLayout = layout;
            }
            stage = vk::PipelineStageFlagBits2::eAllCommands;
            access = vk::AccessFlagBits2::eNone;
            family = command::Queue::Impl::families[u32(dst->type)].idx;
            return barrier;
        }
    };

    struct Buffer::Impl final {
        vk::MemoryAllocateFlags static flags;
        vk::BufferUsageFlags2 static usages;

        Barrier barrier;
        vk::UniqueBuffer device_buffer;
        vk::UniqueBuffer host_buffer;

        auto static search(vk::MemoryPropertyFlags flags, u32 heap, u32 type) -> u32;
        auto update(cref<Barrier> desc) noexcept -> vk::BufferMemoryBarrier2;
        auto update(mut<command::Queue> dst, mut<command::Queue> src) noexcept -> vk::BufferMemoryBarrier2;
    };
}
