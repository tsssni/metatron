#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::phase {
    struct Interaction final {
        stsp f;
        fv3 wi;
        f32 pdf;
    };

    MTT_POLY_METHOD(phase_function_sample, sample);

    struct Phase_Function final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<fv3> wo, cref<fv3> wi
    ) const noexcept -> opt<Interaction>>
    ::add_convention<phase_function_sample, auto (
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction>>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};
}
