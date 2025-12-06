#include "buffer.hpp"

namespace mtt::opaque {
    Buffer::Impl::Impl(cref<stl::buf> desc) noexcept {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto flags = vk::BufferUsageFlags{desc.flags
        | u32(vk::BufferUsageFlagBits::eTransferSrc)
        | u32(vk::BufferUsageFlagBits::eTransferDst)
        | u32(vk::BufferUsageFlagBits::eShaderDeviceAddress)};

        if (desc.host) {
            auto import = vk::ImportMemoryHostPointerInfoEXT{
                .handleType = vk::ExternalMemoryHandleTypeFlagBits::eHostAllocationEXT,
                .pHostPointer = desc.host,
            };
            auto alloc = vk::MemoryAllocateInfo{
                .pNext = &import,
                .allocationSize = desc.bytelen,
                .memoryTypeIndex = ctx->host_memory.type,
            };
            host_memory = command::guard(device.allocateMemoryUnique(alloc));

            auto& host = desc.visible ? buffer : staging;
            host = command::guard(device.createBufferUnique({
                .size = desc.bytelen,
                .usage = flags,
            }));
            command::guard(device.bindBufferMemory(host.get(), host_memory.get(), 0));
        }

        if (!desc.visible && desc.bytelen > 0) {
            auto alloc = vk::MemoryAllocateInfo{
                .allocationSize = desc.bytelen,
                .memoryTypeIndex = ctx->device_memory.type,
            };
            device_memory = command::guard(device.allocateMemoryUnique(alloc));

            buffer = command::guard(device.createBufferUnique({
                .size = desc.bytelen,
                .usage = flags,
            }));
            command::guard(device.bindBufferMemory(buffer.get(), device_memory.get(), 0));
            address = device.getBufferAddress({.buffer = buffer.get()});
        }
    }

    Buffer::Impl::Impl(ref<stl::buf> buffer) noexcept
    : Impl(cref<stl::buf>(buffer)) { buffer.device = address; }
    Buffer::Buffer(cref<stl::buf> buffer) noexcept {}
    Buffer::Buffer(ref<stl::buf> buffer) noexcept {}
}
