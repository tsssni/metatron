#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    namespace filter {
        struct Interaction final {
            Vector<f32, 2> p;
            f32 weight;
            f32 pdf;
        };
    }

    MTT_POLY_METHOD(filter_sample, sample);

    struct Filter final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        Vector<f32, 2> const& p
    ) const noexcept -> f32>
    ::add_convention<filter_sample, auto (
        Vector<f32, 2> const& u
    ) const -> std::optional<filter::Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
