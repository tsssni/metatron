#include <metatron/volume/media/nanovdb.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/grid/uniform-grid.hpp>
#include <metatron/core/stl/optional.hpp>
#include <nanovdb/math/SampleFromVoxels.h>
#include <nanovdb/math/Ray.h>

namespace metatron::media {
	using Nanovdb_Grid = math::Uniform_Grid<f32, 64, 64, 64>;
	using Nanovdb_Sampler = nanovdb::math::SampleFromVoxels<nanovdb::FloatGrid::TreeType, 1, false>;

	namespace {
		template<typename T, usize n>
		auto to_nanovdb(math::Vector<T, n> x) {
			if constexpr (n == 3) {
				return nanovdb::math::Vec3<T>{x[0], x[1], x[2]};
			} else if constexpr (n == 4) {
				return nanovdb::math::Vec4<T>{x[0], x[1], x[2], x[3]};
			} else {
				std::printf("not supported to be converted to nanovdb vec");
				std::abort();
			}
		}

		template<typename T>
		auto from_nanovdb(nanovdb::math::Vec3<T> const& x) -> math::Vector<T, 3> {
			return {x[0], x[1], x[2]};
		}

		template<typename T>
		auto from_nanovdb(nanovdb::math::Vec4<T> const& x) -> math::Vector<T, 4> {
			return {x[0], x[1], x[2], x[3]};
		}

		auto from_nanovdb(nanovdb::Vec3dBBox const& bbox) {
			return math::Bounding_Box{
				from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[0]}),
				from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[1]}),
			};
		}
	}

	std::unordered_map<Nanovdb_Medium const*, Nanovdb_Medium::Cache> thread_local Nanovdb_Medium::thread_caches;

    Nanovdb_Medium::Nanovdb_Medium(
        std::string_view path,
        std::unique_ptr<spectra::Spectrum> sigma_a,
        std::unique_ptr<spectra::Spectrum> sigma_s,
        std::unique_ptr<spectra::Spectrum> Le,
		std::unique_ptr<phase::Phase_Function> phase,
        f32 density_scale
    ):
	sigma_a(std::move(sigma_a)),
	sigma_s(std::move(sigma_s)),
	Le(std::move(Le)),
	phase(std::move(phase)),
	density_scale(density_scale),
	handle(nanovdb::io::readGrid(std::string{path})),
	nanovdb_grid(handle.grid<f32>()) {
		auto& cache = thread_caches[this];
		auto& root = nanovdb_grid->tree().root();
		auto accessor = nanovdb_grid->getAccessor();
		cache.density_inactive = root.background();

		grid = std::make_unique<Nanovdb_Grid>(from_nanovdb(nanovdb_grid->worldBBox()));
		for (auto n = 0; n < grid->dimensions[0] * grid->dimensions[1] * grid->dimensions[2]; n++) {
			auto ijk = math::Vector<i32, 3>{
				n / (grid->dimensions[2] * grid->dimensions[1]),
				(n / grid->dimensions[2]) % grid->dimensions[1],
				n % grid->dimensions[2]
			};
			auto voxel_bbox = grid->bounding_box(ijk);
			(*grid)[ijk] = math::low<f32>;

			auto coord_bbox = from_nanovdb(nanovdb_grid->indexBBox());
			auto coord_min = from_nanovdb(nanovdb_grid->worldToIndexF(to_nanovdb(voxel_bbox.p_min)));
			auto coord_max = from_nanovdb(nanovdb_grid->worldToIndexF(to_nanovdb(voxel_bbox.p_max)));
			coord_min = math::clamp(coord_min - 1, coord_bbox.p_min, coord_bbox.p_max);
			coord_max = math::clamp(coord_max + 1, coord_bbox.p_min, coord_bbox.p_max);

			auto pmin = math::Vector<i32, 3>{coord_min};
			auto pmax = math::Vector<i32, 3>{coord_max};
			for (auto i = pmin[0]; i <= pmax[0]; i++) {
				for (auto j = pmin[1]; j <= pmax[1]; j++) {
					for (auto k = pmin[2]; k <= pmax[2]; k++) {
						(*grid)[ijk] = std::max((*grid)[ijk], accessor.getValue({i, j, k}));
					}
				}
			}
		}
	}

	Nanovdb_Medium::~Nanovdb_Medium() {
		thread_caches.erase(this);
	}
    
    auto Nanovdb_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const -> std::optional<Interaction> {
		auto sampler = Nanovdb_Sampler(nanovdb_grid->tree());
		auto& cache = thread_caches[this];
		
		auto sigma_a = ctx.L & (*this->sigma_a);
		auto sigma_s = ctx.L & (*this->sigma_s);
		auto sigma_t = sigma_a + sigma_s;

		auto update_majorant = [&]() -> void {
			cache.bbox = grid->bounding_box(cache.r.o);
			cache.t_max = math::hit(cache.r, cache.bbox).value_or(-1.f);
			if (cache.t_max < 0.f) {
				cache.t_max = t_max;
				cache.density_maj = cache.density_inactive;
			} else if (!inside(cache.r.o, grid->bounding_box())) {
				cache.density_maj = cache.density_inactive;
			} else {
				cache.density_maj = (*grid)(cache.r.o);
			}
			cache.sigma_maj = cache.density_maj * sigma_t;
			cache.distr = math::Exponential_Distribution(cache.sigma_maj.value.front());
		};

		if (false 
		|| math::length(ctx.r.o - cache.r.o) > 1e-3f
		|| math::length(ctx.r.d - cache.r.d) > 1e-3f) {
			cache.r = ctx.r;
			update_majorant();
		}

		auto t_boundary = t_max;
		auto t_transmitted = 0.f;
		auto transmittance = ctx.L;
		transmittance.value = std::vector<f32>(transmittance.lambda.size(), 1.f);
		auto update_transmittance = [&](f32 t) -> void {
			t_transmitted += t;
			t_boundary -= t;
			cache.t_max -= t;
			cache.r.o += t * cache.r.d;
			for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
				transmittance.value[i] *= std::exp(-cache.sigma_maj.value[i] * t);
			}
		};

		while (true) {
			auto t_u = cache.distr.sample(u);
			if (t_boundary <= cache.t_max && (cache.density_maj == 0.f || t_u >= t_boundary)) {
				update_transmittance(t_boundary + 0.001f);
				return Interaction{
					cache.r.o,
					phase.get(),
					t_max,
					transmittance.value.front(),
					transmittance,
					transmittance,
					{}, {}, {}, {}, {}
				};
			} else if (t_boundary > cache.t_max && (cache.density_maj == 0.f || t_u >= cache.t_max)) {
				update_transmittance(cache.t_max + 0.001f);
				update_majorant();
			} else {
				update_transmittance(t_u);
				auto spectra_pdf = cache.sigma_maj * transmittance;
				auto idx = nanovdb_grid->worldToIndex(to_nanovdb(cache.r.o));
				auto density = sampler(idx) * density_scale;

				return Interaction{
					cache.r.o,
					phase.get(),
					t_transmitted,
					spectra_pdf.value.front(),
					spectra_pdf,
					transmittance,
					density * sigma_a,
					density * sigma_s,
					cache.sigma_maj - density * sigma_t,
					cache.sigma_maj,
					density * (ctx.L & *Le),
				};
			}
		}
    }
}
