#pragma once
#include <metatron/render/spectra/interaction.hpp>
#include <string>

namespace mtt::render {

struct Visible_Spectrum final {
    buf<f32> storage;
    Visible_Spectrum(std::string const& path) noexcept;
    auto operator()(f32 lambda) const noexcept -> f32;
};

}
