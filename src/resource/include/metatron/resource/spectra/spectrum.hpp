#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/vector.hpp>
#include <unordered_map>

namespace mtt::spectra {
    auto constexpr visible_lambda = fv2{360.f, 830.f};
    auto constexpr CIE_Y_integral = 106.7502593994140625f;

    struct Spectrum final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (f32) const noexcept -> f32>
    ::add_skill<pro::skills::as_view>
    ::build {
        // IOR data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/ior
        // CIE data: https://github.com/mmp/pbrt-v4/tree/master/src/pbrt/util/spectrum.cpp
    };

    auto inline operator|(tag<Spectrum> x, tag<Spectrum> y) noexcept -> f32 {
        auto integral = 0.f;
        auto z = x.data();
        auto w = y.data();
        for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
            integral += (*z)(lambda) * (*w)(lambda);
        return integral;
    }

    auto inline operator~(tag<Spectrum> s) noexcept -> fv3 {
        return fv3{
            entity<Spectrum>("/spectrum/CIE-X") | s,
            entity<Spectrum>("/spectrum/CIE-Y") | s,
            entity<Spectrum>("/spectrum/CIE-Z") | s,
        };
    }

    auto inline operator&(cref<fv4> lambda, view<Spectrum> s) noexcept -> fv4 {
        if (math::constant(lambda)) return fv4{(*s)(lambda[0])};
        return math::foreach([&](f32 lambda, auto) {
            return (*s)(lambda);
        }, lambda);
    }

    auto inline operator&(cref<fv4> lambda, tag<Spectrum> s) noexcept -> fv4 {
        return lambda & s.data();
    }

    template<typename Func>
    auto inline visit(Func f, cref<fv4> lambda) noexcept -> fv4 {
        if (math::constant(lambda)) return fv4{f(lambda[0], 0)};
        else return math::foreach(f, lambda);
    }
}
