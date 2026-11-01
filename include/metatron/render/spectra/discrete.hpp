#pragma once
#include <string>

namespace mtt::render {

struct Discrete_Spectrum final {
    usize count = 0;
    buf<f32> lambda;
    buf<f32> storage;
    Discrete_Spectrum(std::string const& path) noexcept;
    auto operator()(f32 lambda) const noexcept -> f32;
};

}
