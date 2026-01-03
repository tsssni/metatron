#include "allocator.hpp"

namespace mtt::command {
    Memory::Memory(u32 type, u32 flags) noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto desc = MTL::HeapDescriptor::alloc()->init();
        desc->setSize(memory_size);
        desc->setType(MTL::HeapTypePlacement);
        switch (Impl::Type(type)) {
        case Impl::Type::local: desc->setStorageMode(MTL::StorageModePrivate); break;
        case Impl::Type::visible: desc->setStorageMode(MTL::StorageModeShared); break;
        }
        impl->heap = device->newHeap(desc);
        ctx->residency->addAllocation(impl->heap.get());
        ctx->residency->commit();
    }
    Memory::~Memory() noexcept {}

    Allocator::Allocator() noexcept {
        auto& ctx = Context::instance().impl;
        heaps.resize(num_heaps);
        offsets.resize(heaps.size());
        locks.resize(heaps.size());
        for (auto& f: locks) f = make_obj<std::atomic_flag>(0);
    }
}
