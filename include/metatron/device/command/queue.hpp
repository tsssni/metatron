#pragma once
#include <metatron/device/command/timeline.hpp>
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
        auto allocate(Pairs&& waits) noexcept -> std::unique_ptr<Buffer>;
        auto submit(std::unique_ptr<Buffer>&& cmd, Pairs&& signals) noexcept -> void;
    };
}
