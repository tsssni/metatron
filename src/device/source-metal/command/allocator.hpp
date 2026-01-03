#pragma once
#include "context.hpp"
#include <metatron/device/command/allocator.hpp>

namespace mtt::command {
    struct Memory::Impl final {
        enum struct Type: u32 {
            local = 0,
            visible = 1,
        };
        mtl<MTL::Heap> heap;
    };
    auto constexpr num_heaps = 2;
}
