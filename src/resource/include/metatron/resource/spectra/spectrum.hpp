#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/vector.hpp>
#include <unordered_map>

namespace mtt::spectra {
    auto constexpr visible_lambda = math::Vector<f32, 2>{360.f, 830.f};
    auto constexpr CIE_Y_integral = 106.7502593994140625f;

    struct Spectrum final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (f32) const noexcept -> f32>
    ::add_skill<pro::skills::as_view>
    ::build {
        // IOR data: https://github.com/mitsuba-renderer/mitsuba-data/tree/master/ior
        // CIE data: https://github.com/mmp/pbrt-v4/tree/master/src/pbrt/util/spectrum.cpp
        std::unordered_map<std::string, proxy<Spectrum>> static spectra;
    };

    auto inline operator|(proxy<Spectrum> x, proxy<Spectrum> y) noexcept -> f32 {
        auto integral = 0.f;
        auto a = x.data();
        auto b = y.data();
        for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; ++lambda)
            integral += (*a)(lambda) * (*b)(lambda);
        return integral;
    }

    auto inline operator~(proxy<Spectrum> s) noexcept -> math::Vector<f32, 3> {
        return math::Vector<f32, 3>{
            Spectrum::spectra["CIE-X"] | s,
            Spectrum::spectra["CIE-Y"] | s,
            Spectrum::spectra["CIE-Z"] | s,
        };
    }
}
