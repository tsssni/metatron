#pragma once
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::opaque {
    struct Buffer final: stl::capsule<Buffer> {
        struct Impl;
        Buffer(usize size) noexcept;
        Buffer(ref<stl::buf> buf) noexcept;
    };
}
