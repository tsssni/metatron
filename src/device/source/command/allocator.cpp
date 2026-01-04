#include <metatron/device/command/allocator.hpp>
#include <metatron/core/math/bit.hpp>

namespace mtt::command {
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
