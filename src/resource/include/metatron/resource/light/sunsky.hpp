#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/distribution/gaussian.hpp>
#include <metatron/core/math/distribution/cone.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::light {
    struct Sunsky_Light final: stl::capsule<Sunsky_Light> {
        struct Impl;
        Sunsky_Light(
            math::Vector<f32, 2> direction,
            f32 turbidity,
            f32 albedo,
            f32 aperture,
            f32 temperature,
            f32 intensity
        ) noexcept;

        auto static init() noexcept -> void;

        // Hosek atomosphere model: https://cgg.mff.cuni.cz/projects/SkylightModelling/
        // binary data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/sunsky/output
        auto operator()(
            eval::Context const& ctx
        ) const noexcept -> std::optional<Interaction>;
        // TGMM sky sampling: https://diglib.eg.org/items/b3f1efca-1d13-44d0-ad60-741c4abe3d21
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
