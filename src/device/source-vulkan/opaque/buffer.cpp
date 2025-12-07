#include "buffer.hpp"

namespace mtt::opaque {
    vk::MemoryAllocateFlags Buffer::Impl::flags = vk::MemoryAllocateFlags{}
    | vk::MemoryAllocateFlagBits::eDeviceAddress;
    vk::BufferUsageFlags2 Buffer::Impl::usages = vk::BufferUsageFlags2{}
    | vk::BufferUsageFlagBits2::eTransferSrc
    | vk::BufferUsageFlagBits2::eTransferDst
    | vk::BufferUsageFlagBits2::eShaderDeviceAddress;

    vk::BufferUsageFlags2 static usages;
    Buffer::Buffer(cref<Descriptor> desc) noexcept:
    size(desc.size),
    state(desc.state),
    type(desc.type) {
        if (desc.size == 0) stl::abort("empty buffer not supported");

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto& queue = command::Queues::instance().queues[u32(type)];
        auto flags = vk::MemoryAllocateFlagsInfo{.flags = Impl::flags,};
        auto usages = vk::BufferUsageFlags2CreateInfo{.usage = {
            vk::BufferUsageFlags2{desc.flags} | impl->usages
        }};
        auto create = vk::BufferCreateInfo{
            .pNext = &usages,
            .size = desc.size,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        };

        auto infos = std::array<vk::BindBufferMemoryInfo, 2>{};
        auto& device_info = infos[0];
        auto& host_info = infos[1];

        if (desc.state != State::local) {
            auto alloc = vk::MemoryAllocateInfo{
                .pNext = &flags,
                .allocationSize = desc.size,
                .memoryTypeIndex = ctx->host_memory,
            };
            impl->host_memory = command::guard(device.allocateMemoryUnique(alloc));
            impl->host_buffer = command::guard(device.createBufferUnique(create));
            host_info = {
                .buffer = impl->host_buffer.get(),
                .memory = impl->host_memory.get(),
                .memoryOffset = 0,
            };
        }

        if (desc.state != State::visible) {
            auto alloc = vk::MemoryAllocateInfo{
                .pNext = flags,
                .allocationSize = desc.size,
                .memoryTypeIndex = ctx->device_memory,
            };
            impl->device_memory = command::guard(device.allocateMemoryUnique(alloc));
        }

        impl->device_buffer = command::guard(device.createBufferUnique(create));
        device_info = {
            .buffer = impl->device_buffer.get(),
            .memory = desc.state == State::visible
            ? impl->host_memory.get() : impl->device_memory.get(),
            .memoryOffset = 0,
        };

        command::guard(device.bindBufferMemory2(infos[1].buffer ? 2 : 1, infos.data()));
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

    Buffer::~Buffer() noexcept {
        if (!ptr) return;
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        command::guard(device.unmapMemory2({.memory = impl->host_memory.get()}));
    }

    auto Buffer::upload() noexcept -> obj<Buffer> {
        auto uploaded = make_obj<Buffer>();
        uploaded->impl->host_memory = std::move(impl->host_memory);
        uploaded->impl->host_buffer = std::move(impl->host_buffer);
        uploaded->type = type;
        uploaded->timestamp = timestamp;
        uploaded->state = State::visible;
        uploaded->ptr = ptr;
        uploaded->size = size;

        auto& ctx = command::Context::instance().impl;
        auto& queue = command::Queues::instance().queues[u32(uploaded->type)];
        auto device = ctx->device.get();
        auto usages = vk::BufferUsageFlags2CreateInfo{.usage = Impl::usages, };

        auto create = vk::BufferCreateInfo{
            .pNext = &usages,
            .size = uploaded->size,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        };
        uploaded->impl->device_buffer = command::guard(device.createBufferUnique(create));

        auto info = vk::BindBufferMemoryInfo{
            .buffer = uploaded->impl->device_buffer.get(),
            .memory = uploaded->impl->host_memory.get(),
            .memoryOffset = 0,
        };
        command::guard(device.bindBufferMemory2(1, &info));
        uploaded->addr = device.getBufferAddress({
            .buffer = uploaded->impl->device_buffer.get(),
        });
        ptr = nullptr;
        return uploaded;
    }

    auto Buffer::flush(view<Buffer> dst, usize offset) noexcept -> obj<Buffer> {
        auto flushed = make_obj<Buffer>();
        flushed->type = type;
        flushed->state = State::visible;
        flushed->timestamp = timestamp;
        flushed->ptr = dst->ptr + offset;
        flushed->size = dirty[1] - dirty[0];
        std::memcpy(flushed->ptr, ptr + dirty[0], flushed->size);

        auto& ctx = command::Context::instance().impl;
        auto& queue = command::Queues::instance().queues[u32(flushed->type)];
        auto device = ctx->device.get();
        auto usages = vk::BufferUsageFlags2CreateInfo{.usage = Impl::usages, };
        
        auto info = vk::BufferCreateInfo{
            .pNext = &usages,
            .size = flushed->size,
            .sharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        };
        flushed->impl->host_buffer = command::guard(device.createBufferUnique(info));
        flushed->impl->device_buffer = command::guard(device.createBufferUnique(info));

        auto infos = std::array<vk::BindBufferMemoryInfo, 2>{};
        auto& device_info = infos[0];
        auto& host_info = infos[1];
        device_info = {
            .buffer = flushed->impl->host_buffer.get(),
            .memory = dst->impl->host_memory.get(),
            .memoryOffset = offset,
        };
        host_info = {
            .buffer = flushed->impl->device_buffer.get(),
            .memory = dst->impl->host_memory.get(),
            .memoryOffset = offset,
        };
        command::guard(device.bindBufferMemory2(2, infos.data()));

        flushed->addr = device.getBufferAddress({
            .buffer = flushed->impl->device_buffer.get()
        });
        dirty = {math::maxv<u32>, math::minv<u32>};
        return flushed;
    }
}
