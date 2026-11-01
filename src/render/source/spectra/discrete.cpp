#include <metatron/render/spectra/discrete.hpp>
#include <fstream>
#include <sstream>
#include <vector>

namespace mtt::render {

Discrete_Spectrum::Discrete_Spectrum(std::string const& path) noexcept {
    auto file = std::ifstream{path};
    auto line = std::string{};
    auto data = std::vector<fv2>{};
    data.reserve(256);

    while (std::getline(file, line)) {
        if (line.empty() || line.front() == '#') continue;
        auto value = fv2{};
        if (std::istringstream{line} >> value[0] >> value[1]) data.push_back(value);
    }

    count = data.size();
    lambda = count;
    storage = count;
    lambda.map();
    storage.map();
    for (auto i = 0uz; i < count; ++i) {
        lambda[i] = data[i][0];
        storage[i] = data[i][1];
    }
    lambda.unmap();
    storage.unmap();
    file.close();
}

auto Discrete_Spectrum::operator()(f32 wavelength) const noexcept -> f32 {
    if (count == 1) return storage[0];
    auto size = i32(count);
    wavelength = core::clamp(wavelength, lambda[0], lambda[size - 1]);

    auto span = std::span<f32 const>{lambda.ptr, count};
    auto idx = core::clamp(
        i32(std::ranges::lower_bound(span, wavelength) - span.begin()) - 1,
        0, size - 2
    );

    auto alpha = (wavelength - lambda[idx]) / (lambda[idx + 1] - lambda[idx]);
    return core::lerp(storage[idx], storage[idx + 1], alpha);
}

}
