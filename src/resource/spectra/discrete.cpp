#include <metatron/resource/spectra/discrete.hpp>
#include <algorithm>

namespace mtt::spectra {
    Discrete_Spectrum::Discrete_Spectrum(std::vector<f32>&& lambda, std::vector<f32>&& data) noexcept
    : lambda(std::move(lambda)), data(std::move(data)) {}

    Discrete_Spectrum::Discrete_Spectrum(std::vector<math::Vector<f32, 2>>&& interleaved) noexcept
    : lambda(interleaved.size()), data(interleaved.size()) {
        for (auto i = 0uz; i < interleaved.size(); ++i) {
            lambda[i] = interleaved[i][0];
            data[i] = interleaved[i][1];
        }
    }

    auto Discrete_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        if (lambda < this->lambda.front() || lambda > this->lambda.back()) return 0.f;

        auto idx = std::max(0uz, std::min(
            this->lambda.size() - 2,
            std::lower_bound(
                this->lambda.begin(),
                this->lambda.end(),
                lambda
            ) - this->lambda.begin() - 1uz
        ));
        auto alpha = (lambda - this->lambda[idx]) / (this->lambda[idx + 1] - this->lambda[idx]);
        return math::lerp(data[idx], data[idx + 1], alpha);
    }
}
