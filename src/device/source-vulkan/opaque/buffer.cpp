#include "buffer.hpp"
#include "../command/queue.hpp"
#include "../command/allocator.hpp"

namespace mtt::opaque {
    vk::MemoryAllocateFlags Buffer::Impl::flags = vk::MemoryAllocateFlags{}
    | vk::MemoryAllocateFlagBits::eDeviceAddress;
    vk::BufferUsageFlags2 Buffer::Impl::usages = vk::BufferUsageFlags2{}
    | vk::BufferUsageFlagBits2::eTransferSrc
    | vk::BufferUsageFlagBits2::eTransferDst
    | vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR
    | vk::BufferUsageFlagBits2::eShaderDeviceAddress;

    auto Buffer::Impl::search(vk::MemoryPropertyFlags flags, u32 heap, u32 type) -> u32 {
        auto& ctx = command::Context::instance().impl;
        auto i = 0u;
        while (type > 0) {
            if (!(type & 1)) {
                ++i; type >>= 1;
                continue;
            }
            auto mem = ctx->memory_props.memoryProperties.memoryTypes[i];
            if (mem.heapIndex == heap && mem.propertyFlags & flags) return i;
            ++i; type >>= 1;
        }
        stl::abort("no valid memory on heap {} with flags {:x}", heap, u32(flags));
        return math::maxv<u32>;
    }

    auto Buffer::Impl::update(cref<Barrier> desc) noexcept -> vk::BufferMemoryBarrier2 {
        auto barrier = this->barrier.update<vk::BufferMemoryBarrier2>(desc);
        barrier.buffer = device_buffer.get();
        barrier.offset = 0;
        barrier.size = vk::WholeSize;
        return barrier;
    }

    Buffer::Buffer(cref<Descriptor> desc) noexcept:
    size(desc.size),
    state(desc.state) {
        impl->barrier.family = command::Queue::Impl::families[u32(desc.type)].idx;
        auto& ctx = command::Context::instance().impl;
        auto& props = ctx->memory_props;
        auto& allocator = command::Allocator::instance();
        if (desc.size == 0) stl::abort("empty buffer not supported");

        auto device = ctx->device.get();
        auto usages = vk::BufferUsageFlags2CreateInfo{.usage = {
            vk::BufferUsageFlags2{desc.flags} | impl->usages
        }};
        auto create = vk::BufferCreateInfo{
            .pNext = &usages,
            .size = desc.size,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &impl->barrier.family,
        };

        impl->device_buffer = command::guard(device.createBufferUnique(create));
        auto reqs = device.getBufferMemoryRequirements2({
            .buffer = impl->device_buffer.get(),
        }).memoryRequirements;

        auto infos = std::array<vk::BindBufferMemoryInfo, 2>{};
        auto& device_info = infos[0];
        auto& host_info = infos[1];
        auto device_alloc = command::Allocation{};
        auto host_alloc = command::Allocation{};

        if (desc.state != State::local || desc.ptr) {
            impl->host_buffer = command::guard(device.createBufferUnique(create));
            auto type = impl->search(
                vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                ctx->host_heap, reqs.memoryTypeBits
            );
            host_alloc = allocator.allocate(
                type, u32(Impl::flags),
                math::max(desc.alignment, reqs.alignment),
                reqs.size
            );
            host_info = {
                .buffer = impl->host_buffer.get(),
                .memory = host_alloc.memory->impl->memory.get(),
                .memoryOffset = host_alloc.offset,
            };
        }

        if (desc.state != State::visible) {
            auto type = Impl::search(
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                ctx->device_heap, reqs.memoryTypeBits
            );
            device_alloc = allocator.allocate(
                type, u32(Impl::flags),
                math::max(desc.alignment, reqs.alignment),
                reqs.size
            );
        }
        device_info = {
            .buffer = impl->device_buffer.get(),
            .memory = desc.state == State::visible
            ? host_alloc.memory->impl->memory.get()
            : device_alloc.memory->impl->memory.get(),
            .memoryOffset = desc.state == State::visible
            ? host_alloc.offset : device_alloc.offset,
        };

        command::guard(device.bindBufferMemory2(host_info.buffer ? 2 : 1, infos.data()));
        addr = ctx->device->getBufferAddress({
            .buffer = impl->device_buffer.get()
        });
        if (impl->host_buffer) {
            ptr = host_alloc.memory->mapped + host_alloc.offset;
            if (desc.ptr) std::memcpy(ptr, desc.ptr, desc.size);
        }
    }

    Buffer::Buffer(rref<Buffer> rhs) noexcept { *this = std::move(rhs); }
    auto Buffer::operator=(rref<Buffer> rhs) noexcept -> ref<Buffer> {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        state = rhs.state; ptr = rhs.ptr;
        addr = rhs.addr; size = rhs.size; dirty = std::move(rhs.dirty);
        impl = std::move(rhs.impl);
        rhs.ptr = nullptr;
        rhs.addr = 0;
        return *this;
    }

    Buffer::operator View() noexcept {
        return {this, 0, size};
    }
}
