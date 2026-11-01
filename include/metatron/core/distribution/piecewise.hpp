#pragma once
#include <metatron/core/stl/protocol.hpp>

namespace mtt::core {

struct Piecewise_Distribution final {
    ft2 distr;

    auto update(recorder& recorder, ftw2v weights) noexcept -> void;
    auto sample(fv2 const& u) const noexcept -> fv2;
    auto pdf(fv2 const& u) const noexcept -> f32;
};

auto inline Piecewise_Distribution::update(recorder& recorder, ftw2v weights) noexcept -> void {
    auto size = weights.size();
    size[3] = 0;
    distr.update<ordinary<32>, encoding::sf>(size);
    auto encoder = memonc{recorder};
    encoder.copy(weights, distr);
    encoder.generate_mipmap(distr);
}

auto inline Piecewise_Distribution::sample(fv2 const& u) const noexcept -> fv2 {
    auto size = distr.size();
    auto mip = size[3] - 1;
    if (distr[uzv3{0, 0, mip}][0] == 0.f) return u;

    auto px = uzv3{};
    auto remapped = u;
    auto choose = [](f32& value, f32 first, f32 second) noexcept -> usize {
        auto probability = first / (first + second);
        if (probability == 1.f || value < probability) {
            value /= probability;
            return 0;
        }
        value = (value - probability) / (1.f - probability);
        return 1;
    };

    while (mip > 0) {
        --mip;
        auto extent = distr.size((i32)mip);
        px[0] *= extent[0] > 1 ? 2 : 1;
        px[1] *= extent[1] > 1 ? 2 : 1;
        px[2] = mip;

        auto w00 = distr[px][0];
        auto w10 = extent[0] > 1 ? distr[px + uzv3{1, 0, 0}][0] : 0.f;
        auto w01 = extent[1] > 1 ? distr[px + uzv3{0, 1, 0}][0] : 0.f;
        auto w11 = extent[0] > 1 && extent[1] > 1 ? distr[px + uzv3{1, 1, 0}][0] : 0.f;

        auto x = extent[0] > 1 ? choose(remapped[0], w00 + w01, w10 + w11) : 0uz;
        auto y = extent[1] > 1 ? choose(remapped[1], x == 0 ? w00 : w10, x == 0 ? w01 : w11) : 0uz;
        px[0] += x;
        px[1] += y;
    }

    return (fv2{px} + remapped) / fv2{size};
}

auto inline Piecewise_Distribution::pdf(fv2 const& u) const noexcept -> f32 {
    if (u[0] < 0.f || u[0] >= 1.f || u[1] < 0.f || u[1] >= 1.f) return 0.f;
    auto size = distr.size();
    auto mip = size[3] - 1;
    auto average = distr[uzv3{0, 0, mip}][0];
    if (average == 0.f) return 1.f;
    auto px = uzv3{uzv2{u * fv2{size}}};
    return distr[px][0] / average;
}

}
