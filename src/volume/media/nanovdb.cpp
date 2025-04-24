#include <metatron/volume/media/nanovdb.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>
#include <nanovdb/math/SampleFromVoxels.h>
#include <nanovdb/math/Ray.h>
#include <nanovdb/tools/GridBuilder.h>

namespace metatron::media {
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
	grid(handle.grid<f32>()) {}

	Nanovdb_Medium::~Nanovdb_Medium() {
		thread_caches.erase(this);
	}
    
    auto Nanovdb_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const -> std::optional<Interaction> {
		auto accessor = grid->getAccessor();
		auto& cache = thread_caches[this];
		
		auto sigma_a = ctx.L & (*this->sigma_a);
		auto sigma_s = ctx.L & (*this->sigma_s);
		auto sigma_t = sigma_a + sigma_s;

		auto update_majorant = [&]() -> void {
			auto idx = grid->worldToIndex(to_nanovdb(cache.r.o));
			auto coord = nanovdb::Coord{i32(idx[0]), i32(idx[1]), i32(idx[2])};
			auto node = accessor.getNodeInfo(coord);
			
			auto& bbox = node.bbox;
			cache.bbox.p_min = from_nanovdb(grid->indexToWorld(nanovdb::Vec3f{bbox.min()} - nanovdb::Vec3f{0.5f}));
			cache.bbox.p_max = from_nanovdb(grid->indexToWorld(nanovdb::Vec3f{bbox.max()} + nanovdb::Vec3f{0.5f}));
			cache.t_max = math::hit(cache.r, cache.bbox).value_or(-1.f);

			if (cache.t_max == -1.f) {
				cache.t_max = t_max;
				accessor.probeValue(coord, cache.density_maj);
			} else {
				cache.density_maj = node.maximum;
			}
			cache.sigma_maj = cache.density_maj * sigma_t;
			cache.distr = math::Exponential_Distribution{cache.sigma_maj.value.front()};
		};

		if (false 
		|| ctx.r.o != cache.r.o
		|| ctx.r.d != cache.r.d 
		|| cache.t_max < 0.f) {
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
			cache.r.o += t * cache.r.d;
			for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
				transmittance.value[i] *= std::exp(-cache.sigma_maj.value[i] * t);
			}
		};

		while (true) {
			auto t_u = cache.distr.sample(u);
			if (t_boundary <= cache.t_max && t_u >= t_boundary) {
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
			} else if (t_boundary > cache.t_max && (cache.density_maj == 0.f || t_u > cache.t_max)) {
				update_transmittance(cache.t_max + 0.001f);
				update_majorant();
			} else {
				update_transmittance(t_u);
				auto spectra_pdf = cache.sigma_maj * transmittance;
				auto density = 0.f;
				auto idx = grid->worldToIndex(to_nanovdb(cache.r.o));
				auto coord = nanovdb::Coord{i32(idx[0]), i32(idx[1]), i32(idx[2])};
				accessor.probeValue(coord, density);
				density *= density_scale;

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
