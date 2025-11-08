#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Area_Light final {
        proxy<shape::Shape> shape;
        usize primitive;

        auto operator()(
            math::Ray const& r,
            spectra::Stochastic_Spectrum const& spec
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
