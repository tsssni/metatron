#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>

namespace mtt::daemon {
	auto Texture_Daemon::init() noexcept -> void {
		MTT_SERDE(Texture);
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto conductor_list = std::to_array<std::string>({"Au"});
		for (auto& conductor: conductor_list) {
			hierarchy.attach<compo::Texture>(
				hierarchy.create("/texture/eta/" + conductor),
				compo::Constant_Spectrum_Texture{
					.spectrum = hierarchy.entity("/spectrum/eta/" + conductor),
				}
			);
			hierarchy.attach<compo::Texture>(
				hierarchy.create("/texture/k/" + conductor),
				compo::Constant_Spectrum_Texture{
					.spectrum = hierarchy.entity("/spectrum/k/" + conductor),
				}
			);
		}
	}

	auto Texture_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto texture_view = registry.view<ecs::Dirty_Mark<compo::Texture>>();
		for (auto entity: texture_view) {
			using Spectrum_Texture = texture::Texture<spectra::Stochastic_Spectrum>;
			using Vector_Texture = texture::Texture<math::Vector<f32, 4>>;
			registry.remove<poly<Spectrum_Texture>, poly<Vector_Texture>>(entity);
			if (!registry.any_of<compo::Texture>(entity)) {
				continue;
			}
			auto& texture = registry.get<compo::Texture>(entity);

			std::visit([&](auto&& compo) {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Spectrum_Texture>) {
					registry.emplace<poly<Spectrum_Texture>>(entity,
					std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						using Constant_Texture = texture::Constant_Texture<spectra::Stochastic_Spectrum>;
						using Image_Texture = texture::Image_Texture<spectra::Stochastic_Spectrum>;
						if constexpr (std::is_same_v<T, compo::Constant_Spectrum_Texture>) {
							return make_poly<Spectrum_Texture, Constant_Texture>(
								registry.get<poly<spectra::Spectrum>>(compo.spectrum)
							);
						} else if constexpr (std::is_same_v<T, compo::Image_Spectrum_Texture>) {
							return make_poly<Spectrum_Texture, Image_Texture>(
								image::Image::from_path(compo.path), compo.type
							);
						}
					},compo));
				} else if constexpr (std::is_same_v<T, compo::Vector_Texture>) {
					registry.emplace<poly<Vector_Texture>>(entity,
					std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						using Constant_Texture = texture::Constant_Texture<math::Vector<f32, 4>>;
						using Image_Texture = texture::Image_Texture<math::Vector<f32, 4>>;
						if constexpr (std::is_same_v<T, compo::Constant_Vector_Texture>) {
							return make_poly<Vector_Texture, Constant_Texture>(compo.x);
						} else if constexpr (std::is_same_v<T, compo::Image_Vector_Texture>) {
							return make_poly<Vector_Texture, Image_Texture>(
								image::Image::from_path(compo.path)
							);
						}
					},compo));
				}
			}, texture);
		}

		registry.clear<ecs::Dirty_Mark<compo::Texture>>();
	}
}
