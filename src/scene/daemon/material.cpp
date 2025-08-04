#include <metatron/scene/daemon/material.hpp>
#include <metatron/scene/compo/material.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/bsdf/lambertian.hpp>
#include <metatron/resource/bsdf/microfacet.hpp>

namespace mtt::daemon {
	auto Material_Daemon::init() noexcept -> void {}

	auto Material_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto material_view = registry.view<ecs::Dirty_Mark<compo::Material>>();
		for (auto entity: material_view) {
			registry.remove<material::Material>(entity);
			if (!registry.any_of<compo::Material>(entity)) {
				continue;
			}
			auto& compo = registry.get<compo::Material>(entity);

			auto material = material::Material{};
			material.bsdf = [](compo::Bsdf bsdf) {
				switch (bsdf) {
					case compo::Bsdf::interface:
						return make_poly<bsdf::Bsdf, bsdf::Interface_Bsdf>();
					case compo::Bsdf::lambertian:
						return make_poly<bsdf::Bsdf, bsdf::Lambertian_Bsdf>();
					case compo::Bsdf::microfacet:
						return make_poly<bsdf::Bsdf, bsdf::Microfacet_Bsdf>();
				}
			}(compo.bsdf);

			using Specrum_Texture = texture::Texture<spectra::Stochastic_Spectrum>;
			for (auto& [name, entity]: compo.spectrum_textures) {
				if (!registry.any_of<poly<Specrum_Texture>>(entity)) {
					std::print("no spectrum texture {} in entity {}\n", name, hierarchy.name(entity));
					std::abort();
				}
				material.spectrum_textures.emplace(
					name,
					registry.get<poly<Specrum_Texture>>(entity)
				);
			}

			using Vector_Texture = texture::Texture<math::Vector<f32, 4>>;
			for (auto& [name, entity]: compo.vector_textures) {
				if (!registry.any_of<poly<Vector_Texture>>(entity)) {
					std::print("no vector texture {} in entity {}\n", name, hierarchy.name(entity));
					std::abort();
				}
				material.vector_textures.emplace(
					name,
					registry.get<poly<Vector_Texture>>(entity)
				);
			}

			registry.emplace<material::Material>(entity, material);
		}
		registry.clear<ecs::Dirty_Mark<compo::Material>>();
	}
}
