#pragma once
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <atomic>

namespace mtt::command {
    auto constexpr memory_size = 1 << 28;

    struct Memory final: stl::capsule<Memory> {
        struct Impl;
        Memory(u32 type, u32 flags) noexcept;
        ~Memory() noexcept;
        mut<byte> mapped = nullptr;
    };

    struct Allocation final {
        view<Memory> memory;
        usize offset;
    };

    struct Allocator final: stl::singleton<Allocator> {
        Allocator() noexcept;
        auto allocate(
            u32 type, u32 flags,
            usize alignment, usize size
        ) noexcept -> Allocation;

    private:
        std::vector<std::vector<obj<Memory>>> heaps;
        std::vector<std::vector<usize>> offsets;
        std::vector<obj<std::atomic_flag>> locks;
    };
}
