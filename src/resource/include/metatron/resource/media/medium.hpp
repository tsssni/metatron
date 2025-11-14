#pragma once
#include <metatron/resource/phase/phase-function.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::media {
    struct Interaction final {
        fv3 p;
        obj<phase::Phase_Function> phase;
        f32 t;
        stsp transmittance;
        stsp sigma_a;
        stsp sigma_s;
        stsp sigma_n;
        stsp sigma_maj;
        stsp sigma_e;
    };

    MTT_POLY_METHOD(medium_sample, sample);

    struct Medium final: pro::facade_builder
    ::add_convention<medium_sample, auto (
        cref<eval::Context> ctx, f32 t_max, f32 u
    ) const noexcept -> opt<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};

    struct Phase final {
        enum struct Function: u32 {
            henyey_greenstein,
        } function = Function::henyey_greenstein;
        f32 g = 0.f;

        auto to_phase(
            cref<stsp> spec
        ) const noexcept -> obj<phase::Phase_Function>;
    };
}
