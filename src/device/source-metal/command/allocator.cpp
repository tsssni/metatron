#include "allocator.hpp"

namespace mtt::command {
    Memory::Memory(u32 type, u32 flags) noexcept {}
    Memory::~Memory() noexcept {}
    Allocator::Allocator() noexcept {}
    auto Allocator::allocate(
        u32 type, u32 flags,
        usize alignment, usize size
    ) noexcept -> Allocation { return {}; }
}
