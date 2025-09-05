#include <metatron/scene/daemon/color-space.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::daemon {
	auto Color_Space_Daemon::init() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto name = std::to_array<std::string>({
			"sRGB",
		});
		auto red_primitive = std::to_array<math::Vector<f32, 2>>({
			{0.64f, 0.33f},
		});
		auto green_primitive = std::to_array<math::Vector<f32, 2>>({
			{0.30f, 0.60f},
		});
		auto blue_primitive = std::to_array<math::Vector<f32, 2>>({
			{0.15f, 0.06f},
		});
		auto white_point = std::to_array<std::string>({
			"CIE-D65",
		});
		auto gamma = std::to_array({
			[](f32 x) -> f32 {
				if (x <= 0.0031308f) {
					return 12.92f * x;
				} else {
					return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
				}
			}
		});
		auto linear = std::to_array({
			[](f32 x) -> f32 {
				if (x <= 0.04045f) {
					return x / 12.92f;
				} else {
					return std::pow((x + 0.055f) / 1.055f, 2.4f);
				}
			}
		});

		auto mutex = std::mutex{};
		stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{name.size()}, [&](auto idx) {
			auto i = idx[0];
			auto entity = ("/color-space/" + name[i]) / et;

			auto cs = color::Color_Space{
				name[i],
				red_primitive[i],
				green_primitive[i],
				blue_primitive[i],
				spectra::Spectrum::spectra[white_point[i]],
				gamma[i],
				linear[i]
			};

			{
				auto lock = std::lock_guard(mutex);
				registry.emplace<color::Color_Space>(entity, std::move(cs));
				auto* cs = &hierarchy.fetch<color::Color_Space>(entity);
				color::Color_Space::color_spaces[name[i]] = cs;
			}
		});
	}

	auto Color_Space_Daemon::update() noexcept -> void {}
}
