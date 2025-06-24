#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::bsdf {
	auto Interface_Bsdf::operator()(
		math::Vector<f32, 3> const& wo,
		math::Vector<f32, 3> const& wi
	) const -> std::optional<Interaction> {
		auto f = spectrum;
		f = 1.f;
		return Interaction{f, wo, 1.f};
	}

	auto Interface_Bsdf::sample(
		eval::Context const& ctx,
		math::Vector<f32, 3> const& u
	) const -> std::optional<Interaction> {
		auto f = spectrum;
		f = 1.f;
		return Interaction{f, ctx.r.d, 1.f};
	}

	auto Interface_Bsdf::clone(Attribute const& attr) const -> std::unique_ptr<Bsdf> {
		auto bsdf = std::make_unique<Interface_Bsdf>();
		bsdf->spectrum = attr.spectra.at("spectrum");
		return bsdf;
	}

	auto Interface_Bsdf::flags() const -> Flags {
		return Flags::interface;
	}

	auto Interface_Bsdf::degrade() -> bool {
		return false;
	}
}
