#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/core/stl/print.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace mtt::spectra {
    Discrete_Spectrum::Discrete_Spectrum(cref<Descriptor> desc) noexcept {
        auto file = std::ifstream{desc.path};
        if (!file.is_open())
            stl::abort("failed to open discrete spectrum {}", desc.path);

        auto line = std::string{};
        auto lambda = std::vector<f32>{}; lambda.reserve(256);
        auto storage = std::vector<f32>{}; storage.reserve(256);

        while (std::getline(file, line)) {
            if (line.empty() || line.front() == '#') continue;

            auto iss = std::istringstream{line};
            auto wavelength = 0.f;
            auto value = 0.f;

            if (iss >> wavelength >> value) {
                lambda.push_back(wavelength);
                storage.push_back(value);
            }
        }

        if (lambda.empty())
            stl::abort("empty discrete spectrum not allowed");
        this->lambda = std::span{lambda};
        this->storage = std::span{storage};
    }

    auto Discrete_Spectrum::operator()(f32 wavelength) const noexcept -> f32 {
        if (lambda.size() == 1) return storage[0];
        auto size = i32(lambda.size());
        wavelength = math::clamp(wavelength, lambda[0], lambda[size - 1]);

        auto span = std::span<f32 const>{lambda};
        auto idx = math::clamp(
            i32(std::ranges::lower_bound(span, wavelength) - span.begin()) - 1,
            0, size - 2
        );

        auto alpha = 1.f
        * (wavelength - lambda[idx])
        / (lambda[idx + 1] - lambda[idx]);
        return math::lerp(storage[idx], storage[idx + 1], alpha);
    }
}
