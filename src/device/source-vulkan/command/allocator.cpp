#include "allocator.hpp"

namespace mtt::command {
    Memory::Memory(u32 type, u32 flags) noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto info = vk::MemoryAllocateFlagsInfo{.flags = vk::MemoryAllocateFlags(flags)};
        auto alloc = vk::MemoryAllocateInfo{
            .pNext = &info,
            .allocationSize = memory_size,
            .memoryTypeIndex = type,
        };
        impl->memory = guard(device.allocateMemoryUnique(alloc));

        auto heap = ctx->memory_props.memoryProperties.memoryTypes[type].heapIndex;
        if (heap == ctx->host_heap) mapped = mut<byte>(guard(device.mapMemory2({
            .memory = impl->memory.get(),
            .offset = 0, .size = memory_size,
        })));
    }

    Memory::~Memory() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        if (mapped) guard(device.unmapMemory2({.memory = impl->memory.get()}));
    }

    Allocator::Allocator() noexcept {
        auto& ctx = Context::instance().impl;
        heaps.resize(ctx->memory_props.memoryProperties.memoryTypeCount);
        offsets.resize(heaps.size());
        locks.resize(heaps.size());
        for (auto& f: locks) f = make_obj<std::atomic_flag>(0);
    }
}
