#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::filter {
    struct Lanczos_Filter final {
        struct Descriptor final {
            math::Vector<f32, 2> radius = {0.5f};
            f32 tau = 3.f;
        };
        Lanczos_Filter() noexcept = default;
        Lanczos_Filter(Descriptor const& desc) noexcept;
        auto operator()(math::Vector<f32, 2> const& p) const noexcept -> f32;
        auto sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

    private:
        math::Piecewise_Distribution<2> distribution;
        math::Vector<f32, 2> radius;
        f32 tau;
    };
}
