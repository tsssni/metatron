#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/resource/color/color-space.hpp>

namespace mtt::color {
	extern Color_Space::Scale sRGB_spectrum_z;
	extern Color_Space::Table sRGB_spectrum_table;
}

namespace mtt::daemon {
	Color_Space_Daemon::Color_Space_Daemon(ecs::Hierarchy& hierarchy) noexcept {
		auto& registry = hierarchy.registry;
		registry.emplace<color::Color_Space>(
			hierarchy.create("/color-space/sRGB"),
			math::Vector<f32, 2>{0.64f, 0.33f},
			math::Vector<f32, 2>{0.30f, 0.60f},
			math::Vector<f32, 2>{0.15f, 0.06f},
			spectra::Spectrum::CIE_D65,
			[](f32 x) -> f32 {
				if (x <= 0.0031308f) {
					return 12.92f * x;
				} else {
					return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
				}
			},
			[](f32 x) -> f32 {
				if (x <= 0.04045f) {
					return x / 12.92f;
				} else {
					return std::pow((x + 0.055f) / 1.055f, 2.4f);
				}
			},
			&color::sRGB_spectrum_z,
			&color::sRGB_spectrum_table
		);
	}

	auto Color_Space_Daemon::update(ecs::Hierarchy& hierarchy) noexcept -> void {}
}
