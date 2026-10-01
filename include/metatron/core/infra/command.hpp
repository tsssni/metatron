#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/shell.hpp>
#include <vector>

namespace mtt:: inline infra {
    struct context final {
        auto static init() noexcept -> void;
    };

    struct timeline final {
        timeline() noexcept;
        auto wait(u64 count, u64 timeout = math::maxv<u64>) noexcept -> bool;
        auto signal(u64 count) noexcept -> void;
    };

    struct queue final {
        struct contents;
        queue() noexcept;
        auto reset() noexcept -> void;
    };

    struct recorder final {
        struct contents;
        recorder(queue* q, std::vector<std::tuple<timeline*, u64>>&& ts) noexcept;
        auto submit(std::vector<std::tuple<timeline*, u64>>&& ts) noexcept -> void;
    };
}
