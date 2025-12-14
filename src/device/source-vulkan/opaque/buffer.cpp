#include "buffer.hpp"
#include "../command/buffer.hpp"

namespace mtt::opaque {
    vk::MemoryAllocateFlags Buffer::Impl::flags = vk::MemoryAllocateFlags{}
    | vk::MemoryAllocateFlagBits::eDeviceAddress;
    vk::BufferUsageFlags2 Buffer::Impl::usages = vk::BufferUsageFlags2{}
    | vk::BufferUsageFlagBits2::eTransferSrc
    | vk::BufferUsageFlagBits2::eTransferDst
    | vk::BufferUsageFlagBits2::eShaderDeviceAddress;


    auto Buffer::Impl::update(cref<Barrier> desc) noexcept -> vk::BufferMemoryBarrier2 {
        auto barrier = this->barrier.update<vk::BufferMemoryBarrier2>(desc);
        barrier.buffer = device_buffer.get();
        barrier.offset = 0;
        barrier.size = vk::WholeSize;
        return barrier;
    }

    Buffer::Buffer(cref<Descriptor> desc) noexcept:
    size(desc.size),
    state(desc.state),
    type(desc.cmd->type) {
        impl->barrier.family = desc.cmd->impl->family;
        auto& ctx = command::Context::instance().impl;
        auto& props = ctx->memory_props;
        if (desc.size == 0) stl::abort("empty buffer not supported");

        auto device = ctx->device.get();
        auto flags = vk::MemoryAllocateFlagsInfo{.flags = Impl::flags};
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
        if (desc.state != State::local)
            impl->host_buffer = command::guard(device.createBufferUnique(create));
        auto size = device.getBufferMemoryRequirements2({
            .buffer = impl->device_buffer.get(),
        }).memoryRequirements.size;

        auto infos = std::array<vk::BindBufferMemoryInfo, 2>{};
        auto& device_info = infos[0];
        auto& host_info = infos[1];

        if (desc.state != State::local || desc.ptr) {
            auto alloc = vk::MemoryAllocateInfo{
                .pNext = &flags,
                .allocationSize = size,
                .memoryTypeIndex = ctx->host_memory,
            };
            impl->host_memory = command::guard(device.allocateMemoryUnique(alloc));
            host_info = {
                .buffer = impl->host_buffer.get(),
                .memory = impl->host_memory.get(),
                .memoryOffset = 0,
            };
        }

        if (desc.state != State::visible) {
            auto alloc = vk::MemoryAllocateInfo{
                .pNext = flags,
                .allocationSize = size,
                .memoryTypeIndex = ctx->device_memory,
            };
            impl->device_memory = command::guard(device.allocateMemoryUnique(alloc));
        }
        device_info = {
            .buffer = impl->device_buffer.get(),
            .memory = desc.state == State::visible
            ? impl->host_memory.get() : impl->device_memory.get(),
            .memoryOffset = 0,
        };

        command::guard(device.bindBufferMemory2(host_info.buffer ? 2 : 1, infos.data()));
        addr = ctx->device->getBufferAddress({
            .buffer = impl->device_buffer.get()
        });
        if (impl->host_buffer) {
            ptr = mut<byte>(command::guard(ctx->device->mapMemory2({
                .memory = impl->host_memory.get(),
                .offset = 0, .size = desc.size,
            })));
            if (desc.ptr) std::memcpy(ptr, desc.ptr, desc.size);
        }
    }

    Buffer::Buffer(rref<Buffer> rhs) noexcept { *this = std::move(rhs); }
    auto Buffer::operator=(rref<Buffer> rhs) noexcept -> ref<Buffer> {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        if (impl->host_memory && ptr)
            command::guard(device.unmapMemory2({.memory = impl->host_memory.get()}));
        state = rhs.state; type = rhs.type; ptr = rhs.ptr;
        addr = rhs.addr; size = rhs.size; dirty = std::move(rhs.dirty);
        impl = std::move(rhs.impl);
        rhs.ptr = nullptr;
        rhs.addr = 0;
        return *this;
    }

    Buffer::~Buffer() noexcept {
        if (!impl->host_memory || !ptr) return;
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        command::guard(device.unmapMemory2({.memory = impl->host_memory.get()}));
    }

    Buffer::operator View() noexcept {
        return {this, 0, size};
    }
}
