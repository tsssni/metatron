#include <metatron/volume/media/medium.hpp>
#include <memory>

namespace metatron::media {
	struct Homogeneous_Medium final: Medium {
		Homogeneous_Medium(
			std::unique_ptr<spectra::Spectrum> sigma_a,
			std::unique_ptr<spectra::Spectrum> sigma_s
		);
		auto virtual sample(Context const& ctx, f32 u) const -> std::optional<Interaction>;

	private:
		std::unique_ptr<spectra::Spectrum> sigma_a;
		std::unique_ptr<spectra::Spectrum> sigma_s;
	};
}
