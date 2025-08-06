#include <metatron/scene/daemon/medium.hpp>
#include <metatron/scene/compo/medium.hpp>
#include <metatron/scene/compo/transform.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/media/vaccum.hpp>
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/media/nanovdb.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>

namespace mtt::daemon {
	auto Medium_Daemon::init() noexcept -> void {
		MTT_SERDE(Medium);
		MTT_SERDE(Medium_Instance);
		auto& hierarchy = *ecs::Hierarchy::instance;
		hierarchy.attach<compo::Medium>("/medium/vaccum"_et, compo::Vaccum_Medium{});
		hierarchy.attach("/hierarchy/medium/vaccum"_et, compo::Medium_Instance{
			.path = "/medium/vaccum"_et,
		});
		hierarchy.attach("/hierarchy/medium/vaccum"_et, compo::Transform{});
	}

	auto Medium_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto medium_view = registry.view<ecs::Dirty_Mark<compo::Medium>>();
		for (auto entity: medium_view) {
			registry.remove<
				poly<media::Medium>,
				poly<media::Medium_Grid>
			>(entity);
			if (!registry.any_of<compo::Medium>(entity)) {
				continue;
			}
			auto& medium = registry.get<compo::Medium>(entity);

			registry.emplace<poly<media::Medium>>(entity,
			std::visit([&](auto&& compo) -> poly<media::Medium> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Vaccum_Medium>) {
					return make_poly<media::Medium, media::Vaccum_Medium>();
				} else {
					auto phase = std::visit([](auto&& compo) {
						using T = std::decay_t<decltype(compo)>;
						if constexpr (std::is_same_v<T, compo::Henyey_Greenstein_Phase_Function>) {
							return make_poly<phase::Phase_Function, phase::Henyey_Greenstein_Phase_Function>(
								compo.g
							);
						}
					}, compo.phase);
					if constexpr (std::is_same_v<T, compo::Homogeneous_Medium>) {
						return make_poly<media::Medium, media::Homogeneous_Medium>(
							std::move(phase),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_a),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_s),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_e)
						);
					} else if constexpr (std::is_same_v<T, compo::Grid_Medium>) {
						using Nanovdb_Grid = media::Nanovdb_Grid<
							f32,
							media::grid_size,
							media::grid_size,
							media::grid_size
						>;
						auto const& wd = registry.get<ecs::Working_Directory>(hierarchy.root());
						registry.emplace<poly<media::Medium_Grid>>(entity,
							make_poly<media::Medium_Grid, Nanovdb_Grid>(wd.path + compo.path)
						);
						return make_poly<media::Medium, media::Grid_Medium>(
							registry.get<poly<media::Medium_Grid>>(entity),
							std::move(phase),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_a),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_s),
							registry.get<poly<spectra::Spectrum>>(compo.sigma_e),
							compo.density_scale
						);
					}
				}
			},medium));
		}
		registry.clear<ecs::Dirty_Mark<compo::Medium>>();
	}
}
