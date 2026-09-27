#pragma once
#include <metatron/render/filter/box.hpp>
#include <metatron/render/filter/gaussian.hpp>
#include <metatron/render/filter/lanczos.hpp>

namespace mtt::filter {
    auto init() noexcept -> void;

    struct Filter final: stl::polynomial<Filter
    , Box_Filter
    , Gaussian_Filter
    , Lanczos_Filter> {
        using polynomial::polynomial;

        auto operator()(fv2 const& p) const noexcept -> f32 {
            return visit([&](auto* x) noexcept { return (*x)(p); });
        }
        auto sample(fv2 const& u) const noexcept -> Interaction {
            return visit([&](auto* x) noexcept { return x->sample(u); });
        }
    };
}
