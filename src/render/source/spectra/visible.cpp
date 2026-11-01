#include <metatron/render/spectra/visible.hpp>
#include <metatron/core/math/cmath.hpp>
#include <fstream>
#include <sstream>

namespace mtt::render {

Visible_Spectrum::Visible_Spectrum(std::string const& path) noexcept
: storage((usize)(visible_lambda[1] - visible_lambda[0]) + 1) {
    auto idx = 0;
    auto file = std::ifstream{path};
    storage.map();

    auto line = std::string{};
    while (std::getline(file, line)) {
        if (line.empty() || line.front() == '#') continue;
        if (std::istringstream{line} >> storage[idx]) ++idx;
    }
    storage.unmap();
    file.close();
}

auto Visible_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
    auto idx = core::clamp(usize(core::round(lambda) - visible_lambda[0]), 0uz, (usize)(visible_lambda[1] - visible_lambda[0]));
    return storage[idx];
}

}
