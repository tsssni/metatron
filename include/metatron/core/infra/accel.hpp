#pragma once
#include <metatron/core/infra/memory.hpp>
#include <metatron/core/infra/command.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt:: inline infra {

struct blas final {
    u32 idx = math::maxv<u32>;
    auto destroy() noexcept -> void;
    operator bool() const noexcept { return idx != math::maxv<u32>; }

    struct descriptor final {
        span<fv3, heap::coherent> vertices;
        span<u32, heap::coherent> indices;
    };
    auto update(descriptor const& desc) noexcept -> void;
};

struct tlas final {
    u32 idx = math::maxv<u32>;
    auto destroy() noexcept -> void;
    operator bool() const noexcept { return idx != math::maxv<u32>; }

    struct descriptor final {
        struct instance final {
            blas geometry;
            fm44 transform;
            u8 mask;
        };
        span<instance, heap::coherent> instances;
    };
    auto update(descriptor const& desc) noexcept -> void;
};

struct accelenc final {
    accelenc(recorder& r) noexcept;
    ~accelenc() noexcept;

    auto build(blas target) noexcept -> void;
    auto build(tlas target) noexcept -> void;
};

}
