#include "metatron/resource/texture/texture.hpp"
#include <metatron/scene/daemon/material.hpp>
#include <metatron/scene/compo/material.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::daemon {
	template<typename T>
	auto configure(material::Material& material) noexcept -> void {
		material.configurator = [](bsdf::Attribute const& attr) -> poly<bsdf::Bsdf> {
			auto bsdf = make_poly<bsdf::Bsdf, T>();
			bsdf->configure(attr);
			return bsdf;
		};
	}

	auto Material_Daemon::init() noexcept -> void {
		MTT_SERDE(Material);
	}

	auto Material_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;

		auto view = registry.view<ecs::Dirty_Mark<compo::Material>>()
		| std::views::filter([&](auto entity) {
			registry.remove<poly<material::Material>>(entity);
			return registry.any_of<compo::Material>(entity);
		})
		| std::ranges::to<std::vector<ecs::Entity>>();

		auto mutex = std::mutex{};
		stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{view.size()}, [&](auto idx) {
			auto entity = view[idx[0]];
			auto& compo = registry.get<compo::Material>(entity);

			auto material = material::Material{};
			std::visit([&material](auto&& compo) {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Interface_Bsdf>) {
					return configure<bsdf::Interface_Bsdf>(material);
				} else if constexpr (std::is_same_v<T, compo::Physical_Bsdf>) {
					return configure<bsdf::Physical_Bsdf>(material);
				}
			}, (compo.bsdf));

			for (auto& [name, entity]: compo.spectrum_textures) {
				if (!registry.any_of<poly<texture::Spectrum_Texture>>(entity)) {
					std::println("no spectrum texture {} in entity {}", name, hierarchy.path(entity));
					std::abort();
				}
				material.spectrum_textures.emplace(
					name,
					registry.get<poly<texture::Spectrum_Texture>>(entity)
				);
			}

			for (auto& [name, entity]: compo.vector_textures) {
				if (!registry.any_of<poly<texture::Vector_Texture>>(entity)) {
					std::println("no vector texture {} in entity {}", name, hierarchy.path(entity));
					std::abort();
				}
				material.vector_textures.emplace(
					name,
					registry.get<poly<texture::Vector_Texture>>(entity)
				);
			}

			{
				auto lock = std::lock_guard(mutex);
				registry.emplace<material::Material>(entity, std::move(material));
			}
		});
		registry.clear<ecs::Dirty_Mark<compo::Material>>();
	}
}
