#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/shape/shape.hpp>

namespace mtt::light {
    struct Area_Light final {
        Area_Light(
            view<shape::Shape> shape,
            usize primitive
        ) noexcept;

        auto operator()(
            math::Ray const& r,
            spectra::Stochastic_Spectrum const& spec
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto pdf(
            math::Ray const& r,
            math::Vector<f32, 3> const& np
        ) const noexcept -> f32;
        auto flags() const noexcept -> Flags;

    private:
        view<shape::Shape> shape;
        usize primitive;
    };
}
