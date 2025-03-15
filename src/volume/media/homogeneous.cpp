#include <metatron/volume/media/homogeneous.hpp>

namespace metatron::media {
	Homogeneous_Medium::Homogeneous_Medium(
		std::unique_ptr<spectra::Spectrum> sigma_a,
		std::unique_ptr<spectra::Spectrum> sigma_s
	): sigma_a(std::move(sigma_a)), sigma_s(std::move(sigma_s)) {}

	auto Homogeneous_Medium::sample(Context const& ctx, f32 u) const -> std::optional<Interaction> {
		return Interaction{{}, sigma_a.get(), sigma_s.get()};
	}
}
