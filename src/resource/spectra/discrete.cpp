#include <metatron/resource/spectra/discrete.hpp>
#include <algorithm>
#include <cstring>

namespace mtt::spectra {
    Discrete_Spectrum::Discrete_Spectrum(
        std::span<f32> lambda,
        std::span<f32> data
    ) noexcept: size(lambda.size()) {
        std::memcpy(this->lambda.data(), lambda.data(), size * sizeof(f32));
    }

    Discrete_Spectrum::Discrete_Spectrum(
        std::span<math::Vector<f32, 2>> interleaved
    ) noexcept: size(interleaved.size()) {
        for (auto i = 0uz; i < size; ++i) {
            lambda[i] = interleaved[i][0];
            data[i] = interleaved[i][1];
        }
    }

    auto Discrete_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        if (lambda < this->lambda[0] || lambda > this->lambda[size - 1]) return 0.f;

        auto idx = std::max(0uz, std::min(
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
        return math::lerp(data[idx], data[idx + 1], alpha);
    }
}
