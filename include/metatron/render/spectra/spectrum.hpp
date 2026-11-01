#pragma once
#include <metatron/render/spectra/blackbody.hpp>
#include <metatron/render/spectra/constant.hpp>
#include <metatron/render/spectra/discrete.hpp>
#include <metatron/render/spectra/rgb.hpp>
#include <metatron/render/spectra/visible.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::render {

struct Spectrum final : core::protocol<Constant_Spectrum, Rgb_Spectrum, Blackbody_Spectrum, Visible_Spectrum, Discrete_Spectrum> {
    using protocol::protocol;

    auto operator()(f32 lambda) const noexcept -> f32;
};

auto inline Spectrum::operator()(f32 lambda) const noexcept -> f32 {
    return visit([&](auto* p) noexcept { return (*p)(lambda); });
}

auto constexpr operator|(Spectrum x, Spectrum y) noexcept -> f32 {
    auto integral = 0.f;
    for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
        integral += x(lambda) * y(lambda);
    return integral;
}

auto constexpr operator&(fv4 const& lambda, auto&& s) noexcept -> fv4 {
    if (core::constant(lambda)) return fv4{s(lambda[0])};
    return core::foreach([&](f32 lambda, auto) { return s(lambda); }, lambda);
}

template<typename Func, typename... Args>
auto constexpr visit(Func f, Args&&... lambda) noexcept -> fv4 requires(std::same_as<std::decay_t<Args>, fv4> && ...) {
    if ((core::constant(lambda) && ...)) return fv4{f(lambda[0]..., 0)};
    else return core::foreach(f, lambda...);
}

}
