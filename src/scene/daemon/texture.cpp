#include <metatron/scene/daemon/texture.hpp>
#include <metatron/scene/compo/texture.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
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

		auto view = registry.view<ecs::Dirty_Mark<compo::Texture>>()
		| std::views::filter([&](auto entity) {
			registry.remove<poly<texture::Spectrum_Texture>, poly<texture::Vector_Texture>>(entity);
			return registry.any_of<compo::Texture>(entity);
		})
		| std::ranges::to<std::vector<ecs::Entity>>();

		auto mutex = std::mutex{};
		stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{view.size()}, [&](auto idx) {
			auto entity = view[idx[0]];
			auto& texture = registry.get<compo::Texture>(entity);

			std::visit([&](auto&& compo) {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (stl::is_variant_alternative_v<T, compo::Spectrum_Texture>) {
					auto tex = std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						if constexpr (std::is_same_v<T, compo::Constant_Spectrum_Texture>) {
							return make_poly<texture::Spectrum_Texture, texture::Constant_Spectrum_Texture>(
								registry.get<poly<spectra::Spectrum>>(compo.spectrum)
							);
						} else if constexpr (std::is_same_v<T, compo::Image_Spectrum_Texture>) {
							MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(compo.path), {
								std::println("medium {} not exists", compo.path);
								std::abort();
							});
							return make_poly<texture::Spectrum_Texture, texture::Image_Spectrum_Texture>(
								image::Image::from_path(path), compo.type, compo.anisotropy
							);
						} else if constexpr (std::is_same_v<T, compo::Checkerboard_Texture>) {
							return make_poly<texture::Spectrum_Texture, texture::Checkerboard_Texture>(
								registry.get<poly<spectra::Spectrum>>(compo.x),
								registry.get<poly<spectra::Spectrum>>(compo.y),
								compo.uv_scale
							);
						}
					}, compo::Spectrum_Texture{compo});

					{
						auto lock = std::lock_guard(mutex);
						registry.emplace<poly<texture::Spectrum_Texture>>(entity, std::move(tex));
					}
				} else if constexpr (stl::is_variant_alternative_v<T, compo::Vector_Texture>) {
					auto tex = std::visit([&](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						if constexpr (std::is_same_v<T, compo::Constant_Vector_Texture>) {
							return make_poly<texture::Vector_Texture, texture::Constant_Vector_Texture>(compo.x);
						} else if constexpr (std::is_same_v<T, compo::Image_Vector_Texture>) {
							MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(compo.path), {
								std::println("medium {} not exists", compo.path);
								std::abort();
							});
							return make_poly<texture::Vector_Texture, texture::Image_Vector_Texture>(
								image::Image::from_path(path), compo.anisotropy
							);
						}
					}, compo::Vector_Texture{compo});

					{
						auto lock = std::lock_guard(mutex);
						registry.emplace<poly<texture::Vector_Texture>>(entity, std::move(tex));
					}
				}
			}, texture);
		});

		registry.clear<ecs::Dirty_Mark<compo::Texture>>();
	}
}
