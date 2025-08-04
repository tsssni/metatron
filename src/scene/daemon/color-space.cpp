#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>

namespace mtt::color {
	extern Color_Space::Scale sRGB_spectrum_z;
	extern Color_Space::Table sRGB_spectrum_table;
}

namespace mtt::daemon {
	auto Color_Space_Daemon::init() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		registry.emplace<color::Color_Space>(
			"/color-space/sRGB"_et,
			math::Vector<f32, 2>{0.64f, 0.33f},
			math::Vector<f32, 2>{0.30f, 0.60f},
			math::Vector<f32, 2>{0.15f, 0.06f},
			spectra::Spectrum::spectra["CIE-D65"],
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

		auto cs_list = std::to_array<std::string>({"sRGB"});
		for (auto& name: cs_list) {
			auto entity = hierarchy.entity("/color-space/" + name);
			auto* cs = &hierarchy.fetch<color::Color_Space>(entity);
			color::Color_Space::color_spaces[name] = cs;
		}
	}

	auto Color_Space_Daemon::update() noexcept -> void {}
}
