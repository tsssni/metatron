#include "allocator.hpp"
#include <metatron/core/math/bit.hpp>

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

    auto Allocator::allocate(
        u32 type, u32 flags,
        usize alignment, usize size
    ) noexcept -> Allocation {
        while(locks[type]->test_and_set(std::memory_order::acquire));
        auto idx = 0;
        auto offset = 0uz;

        for (; idx < heaps[type].size(); ++idx) {
            auto aligned = math::align(offsets[type][idx], alignment);
            if (memory_size - aligned >= size) {
                offset = aligned; break;
            }
        }

        if (idx == heaps[type].size()) {
            heaps[type].push_back(make_obj<Memory>(type, flags));
            offsets[type].push_back(0);
        }

        offsets[type][idx] = offset + size;
        locks[type]->clear(std::memory_order::release);
        return {heaps[type][idx].get(), offset};
    }
}
