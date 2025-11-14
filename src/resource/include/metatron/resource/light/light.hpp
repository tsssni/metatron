#pragma once
#include <metatron/resource/eval/context.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::light {
    struct Interaction final {
        stsp L;
        fv3 wi;
        fv3 p;
        f32 t;
        f32 pdf;
    };

    enum Flags {
        delta = 1 << 0,
        inf = 1 << 1,
    };

    MTT_POLY_METHOD(light_sample, sample);
    MTT_POLY_METHOD(light_flags, flags);

    struct Light final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<math::Ray> r, cref<stsp> spec
    ) const noexcept -> opt<Interaction>>
    ::add_convention<light_sample, auto (
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction>>
    ::add_convention<light_flags, auto () const noexcept -> Flags>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
