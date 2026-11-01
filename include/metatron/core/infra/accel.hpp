#pragma once
#include <metatron/core/infra/command.hpp>
#include <metatron/core/infra/memory.hpp>
#include <metatron/core/infra/vector.hpp>

namespace mtt::core {

struct blas final {
    u32 idx = maxv<u32>;
    auto destroy() noexcept -> void;
    operator bool() const noexcept;

    auto update(span<fv3> vertices, span<u32> indices) noexcept -> void;
};

inline blas::operator bool() const noexcept { return idx != maxv<u32>; }

struct tlas final {
    u32 idx = maxv<u32>;
    auto destroy() noexcept -> void;
    operator bool() const noexcept;

    struct instance final {
        blas geometry;
        fm44 transform;
        u8 mask;
    };
    auto update(span<instance> instances) noexcept -> void;
};

inline tlas::operator bool() const noexcept { return idx != maxv<u32>; }

struct accelenc final {
    accelenc(recorder& r) noexcept;
    ~accelenc() noexcept;

    auto build(blas target) noexcept -> void;
    auto build(tlas target) noexcept -> void;
};

}
