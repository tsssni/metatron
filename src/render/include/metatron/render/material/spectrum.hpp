#include <metatron/render/material/texture.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <memory>

namespace metatron::material {
	struct Spectrum_Texture final: Texture<std::unique_ptr<spectra::Spectrum>> {
		auto virtual operator()(intr::Interaction const& intr) -> std::unique_ptr<spectra::Spectrum>;
	};
}
