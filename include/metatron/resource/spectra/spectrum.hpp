#pragma once
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/spectra/blackbody.hpp>
#include <metatron/resource/spectra/visible.hpp>
#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/core/math/eval.hpp>

namespace mtt::spectra {
    struct Spectrum final: stl::polynomial<Spectrum
    , Constant_Spectrum
    , Rgb_Spectrum
    , Blackbody_Spectrum
    , Visible_Spectrum
    , Discrete_Spectrum> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(f32 lambda) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return (*p)(lambda); });
        }
    };

    auto constexpr operator|(Spectrum x, Spectrum y) noexcept -> f32 {
        auto integral = 0.f;
        for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
            integral += x(lambda) * y(lambda);
        return integral;
    }

    auto constexpr operator~(Spectrum s) noexcept -> fv3 {
        return fv3{
            Spectrum::entity("/spectrum/CIE-X") | s,
            Spectrum::entity("/spectrum/CIE-Y") | s,
            Spectrum::entity("/spectrum/CIE-Z") | s,
        };
    }

    auto constexpr operator&(cref<fv4> lambda, auto&& s) noexcept -> fv4 {
        if (math::constant(lambda)) return fv4{s(lambda[0])};
        return math::foreach([&](f32 lambda, auto) {
            return s(lambda);
        }, lambda);
    }

    template<typename Func, typename... Args>
    requires (std::same_as<std::decay_t<Args>, fv4> && ...)
    auto constexpr visit(Func f, Args&&... lambda) noexcept -> fv4 {
        if ((math::constant(lambda) && ...)) return fv4{f(lambda[0]..., 0)};
        else return math::foreach(f, lambda...);
    }
}
