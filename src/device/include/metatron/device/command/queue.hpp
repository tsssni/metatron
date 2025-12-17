#pragma once
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <queue>

namespace mtt::command {
    struct Buffer;

    enum struct Type {
        render,
        transfer,
    };
    auto constexpr static num_types = u32(Type::transfer) + 1;

    struct Queue final: stl::capsule<Queue> {
        Type type;
        struct Impl;
        Queue(Type type) noexcept;
        ~Queue() noexcept;
        auto allocate() noexcept -> obj<Buffer>;
        auto submit(rref<obj<Buffer>> cmd) noexcept -> void;
    };
}
