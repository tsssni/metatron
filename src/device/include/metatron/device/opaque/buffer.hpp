#pragma once
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::opaque {
    struct Buffer final: stl::capsule<Buffer> {
        struct Impl;
        Buffer() noexcept = default;
        Buffer(ref<stl::buf> buffer) noexcept;
        Buffer(cref<stl::buf> buffer) noexcept;
    };
}
