#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/core/stl/print.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace mtt::spectra {
    Discrete_Spectrum::Discrete_Spectrum(cref<Descriptor> desc) noexcept {
        auto idx = 0;
        auto file = std::ifstream{desc.path};
        if (!file.is_open()) {
            std::println("failed to open discrete spectrum {}", desc.path);
            std::abort();
        }
        auto line = std::string{};

        while (std::getline(file, line)) {
            if (line.empty() || line.front() == '#') continue;
            
            auto iss = std::istringstream{line};
            auto wavelength = 0.f;
            auto value = 0.f;

            if (iss >> wavelength >> value) {
                lambda[idx] = wavelength;
                storage[idx] = value;
                ++idx;
            }
        }
        size = idx;
        file.close();
    }

    auto Discrete_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        if (lambda < this->lambda[0] || lambda > this->lambda[size - 1]) return 0.f;

        auto idx = math::max(0uz, math::min(
            size - 2,
            std::lower_bound(
                this->lambda.begin(),
                this->lambda.begin() + size,
                lambda
            ) - this->lambda.begin() - 1uz
        ));
        auto alpha = 1.f
        * (lambda - this->lambda[idx])
        / (this->lambda[idx + 1] - this->lambda[idx]);
        return math::lerp(storage[idx], storage[idx + 1], alpha);
    }
}
