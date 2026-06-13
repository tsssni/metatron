#include <metatron/device/command/allocator.hpp>
#include <metatron/core/math/bit.hpp>

namespace mtt::command {
    auto Allocator::allocate(
        u32 type, u32 flags,
        usize alignment, usize size
    ) noexcept -> Allocation {
        auto& self = instance();
        while(self.locks[type]->test_and_set(std::memory_order::acquire));
        auto idx = 0;
        auto offset = 0uz;

        for (; idx < self.heaps[type].size(); ++idx) {
            auto aligned = math::align(self.offsets[type][idx], alignment);
            if (memory_size - aligned >= size) {
                offset = aligned; break;
            }
        }

        if (idx == self.heaps[type].size()) {
            self.heaps[type].push_back(make_obj<Memory>(type, flags));
            self.offsets[type].push_back(0);
        }

        self.offsets[type][idx] = offset + size;
        self.locks[type]->clear(std::memory_order::release);
        return {self.heaps[type][idx].get(), offset};
    }
}
