#pragma once
#include <metatron/render/filter/interaction.hpp>
#include <metatron/render/filter/box.hpp>
#include <metatron/render/filter/gaussian.hpp>
#include <metatron/render/filter/lanczos.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::filter {
    struct Filter final: stl::polynomial<Filter
    , Box_Filter
    , Gaussian_Filter
    , Lanczos_Filter> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(cref<fv2> p) const noexcept -> f32 {
            return visit([&](auto* x) noexcept { return (*x)(p); });
        }
        auto sample(cref<fv2> u) const noexcept -> opt<Interaction> {
            return visit([&](auto* x) noexcept { return x->sample(u); });
        }
    };
}
