#pragma once
#include <metatron/render/filter/box.hpp>
#include <metatron/render/filter/gaussian.hpp>
#include <metatron/render/filter/lanczos.hpp>

namespace mtt::render {

struct Filter final : core::protocol<Box_Filter, Gaussian_Filter, Lanczos_Filter> {
    using protocol::protocol;

    auto operator()(fv2 const& p) const noexcept -> f32;
    auto sample(fv2 const& u) const noexcept -> Filter_Interaction;
};

auto inline Filter::operator()(fv2 const& p) const noexcept -> f32 {
    return visit([&](auto* x) noexcept { return (*x)(p); });
}

auto inline Filter::sample(fv2 const& u) const noexcept -> Filter_Interaction {
    return visit([&](auto* x) noexcept { return x->sample(u); });
}

}
