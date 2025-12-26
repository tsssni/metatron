#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::filter {
    struct Interaction final {
        fv2 p;
        f32 weight;
        f32 pdf;
    };

    MTT_POLY_METHOD(filter_sample, sample);

    struct Filter final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (cref<fv2> p) const noexcept -> f32>
    ::add_convention<filter_sample, auto (cref<fv2> u) const -> opt<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
