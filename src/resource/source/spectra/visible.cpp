#include <metatron/resource/spectra/visible.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>
#include <sstream>

namespace mtt::spectra {
    Visible_Spectrum::Visible_Spectrum(cref<Descriptor> desc) noexcept {
        auto idx = 0;
        auto file = std::ifstream{desc.path};
        if (!file.is_open())
            stl::abort("failed to open visible spectrum {}", desc.path);
        auto line = std::string{};

        while (std::getline(file, line)) {
            if (line.empty() || line.front() == '#') continue;

            auto iss = std::istringstream{line};
            auto value = 0.f;

            if (iss >> value) {
                storage[idx] = value;
                ++idx;
            }
        }
        file.close();

    }

    auto Visible_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        auto idx = math::clamp(usize(std::round(lambda) - visible_lambda[0]), 0uz, visible_range - 1uz);
        return storage[idx];
    }
}
