#pragma once
#include <metatron/resource/phase/phase-function.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/eval.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::media {
    struct Interaction final {
        obj<phase::Phase_Function> phase;
        fv3 p;
        f32 t;
        fv4 transmittance;
        fv4 sigma_a;
        fv4 sigma_s;
        fv4 sigma_n;
        fv4 sigma_maj;
        fv4 sigma_e;
    };

    MTT_POLY_METHOD(iterator_march, march);
    MTT_POLY_METHOD(medium_begin, begin);

    struct Iterator final: pro::facade_builder
    ::add_convention<iterator_march, auto (f32 u) noexcept -> opt<Interaction>>
    ::build {};


    struct Medium final: pro::facade_builder
    ::add_convention<medium_begin, auto (
        cref<math::Context> ctx, f32 t_max
    ) const noexcept -> obj<Iterator>>
    ::add_skill<pro::skills::as_view>
    ::build {};

    struct Phase final {
        enum struct Function {
            henyey_greenstein,
        } function = Function::henyey_greenstein;
        f32 g = 0.f;

        auto to_phase() const noexcept -> obj<phase::Phase_Function>;
    };
}
