#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::daemon {
	auto Texture_Daemon::init() noexcept -> void {
		MTT_SERDE(Texture);
		auto& hierarchy = *ecs::Hierarchy::instance;
		for (auto& [spec, _]: spectra::Spectrum::spectra) {
			hierarchy.attach<compo::Texture>(
				hierarchy.create("/texture/" + spec),
				compo::Constant_Spectrum_Texture{
					.spectrum = hierarchy.entity("/spectrum/" + spec),
				}
			);
		}
	}

	auto Texture_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto texture_view = registry.view<ecs::Dirty_Mark<compo::Texture>>();
		for (auto entity: texture_view) {
			registry.remove<poly<texture::Spectrum_Texture>, poly<texture::Vector_Texture>>(entity);
			if (!registry.any_of<compo::Texture>(entity)) {
				continue;
			}
			auto& texture = registry.get<compo::Texture>(entity);

			std::visit([&](auto&& compo) {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (stl::is_variant_alternative_v<T, compo::Spectrum_Texture>) {
					registry.emplace<poly<texture::Spectrum_Texture>>(entity,
					std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						if constexpr (std::is_same_v<T, compo::Constant_Spectrum_Texture>) {
							return make_poly<texture::Spectrum_Texture, texture::Constant_Spectrum_Texture>(
								registry.get<poly<spectra::Spectrum>>(compo.spectrum)
							);
						} else if constexpr (std::is_same_v<T, compo::Image_Spectrum_Texture>) {
							MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(compo.path), {
								std::println("medium {} does not exist", compo.path);
								std::abort();
							});
							return make_poly<texture::Spectrum_Texture, texture::Image_Spectrum_Texture>(
								image::Image::from_path(path), compo.type
							);
						} else if constexpr (std::is_same_v<T, compo::Checkerboard_Texture>) {
							return make_poly<texture::Spectrum_Texture, texture::Checkerboard_Texture>(
								registry.get<poly<spectra::Spectrum>>(compo.x),
								registry.get<poly<spectra::Spectrum>>(compo.y),
								compo.uv_scale
							);
						}
					}, compo::Spectrum_Texture{compo}));
				} else if constexpr (stl::is_variant_alternative_v<T, compo::Vector_Texture>) {
					registry.emplace<poly<texture::Vector_Texture>>(entity,
					std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						if constexpr (std::is_same_v<T, compo::Constant_Vector_Texture>) {
							return make_poly<texture::Vector_Texture, texture::Constant_Vector_Texture>(compo.x);
						} else if constexpr (std::is_same_v<T, compo::Image_Vector_Texture>) {
							return make_poly<texture::Vector_Texture, texture::Image_Vector_Texture>(
								image::Image::from_path(compo.path)
							);
						}
					}, compo::Vector_Texture{compo}));
				}
			}, texture);
		}

		registry.clear<ecs::Dirty_Mark<compo::Texture>>();
	}
}
