#pragma once
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/uniform.hpp>
#include <metatron/core/spectra/constant.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <nanovdb/NanoVDB.h>
#include <nanovdb/io/IO.h>
#include <nanovdb/math/SampleFromVoxels.h>
#include <nanovdb/math/Ray.h>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace metatron::media {
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

	auto inline from_nanovdb(nanovdb::Vec3dBBox const& bbox) {
		return math::Bounding_Box{
			from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[0]}),
			from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[1]}),
		};
	}

	template<typename T>
	using Nanovdb_Sampler = nanovdb::math::SampleFromVoxels<nanovdb::NanoTree<T>, 1, false>;

	template<typename T, usize x, usize y, usize z>
	struct Nanovdb_Grid final: math::Grid<f32, x, y, z> {
		Nanovdb_Grid(
			std::string_view path
		):
		handle(nanovdb::io::readGrid(std::string{path})),
		nanovdb_grid(handle.grid<T>()),
		sampler(nanovdb_grid->tree()),
		majorant_grid(from_nanovdb(nanovdb_grid->worldBBox())) {
			auto& root = nanovdb_grid->tree().root();
			auto accessor = nanovdb_grid->getAccessor();
			for (auto n = 0; n < majorant_grid.dimensions[0] * majorant_grid.dimensions[1] * majorant_grid.dimensions[2]; n++) {
				auto ijk = math::Vector<i32, 3>{
					n / (majorant_grid.dimensions[2] * majorant_grid.dimensions[1]),
					(n / majorant_grid.dimensions[2]) % majorant_grid.dimensions[1],
					n % majorant_grid.dimensions[2]
				};
				auto voxel_bbox = majorant_grid.bounding_box(ijk);
				majorant_grid[ijk] = math::low<T>;

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
							majorant_grid[ijk] = std::max(majorant_grid[ijk], accessor.getValue({i, j, k}));
						}
					}
				}
			}
		};

		auto virtual to_local(math::Vector<i32, 3> const& ijk) const -> math::Vector<f32, 3> { return majorant_grid.to_local(ijk); };
		auto virtual to_index(math::Vector<f32, 3> const& pos) const -> math::Vector<i32, 3> { return majorant_grid.to_index(pos); };
		auto virtual bounding_box() const -> math::Bounding_Box { return majorant_grid.bounding_box(); };
		auto virtual bounding_box(math::Vector<f32, 3> const& pos) const -> math::Bounding_Box { return majorant_grid.bounding_box(pos); };
		auto virtual bounding_box(math::Vector<i32, 3> const& ijk) const -> math::Bounding_Box { return majorant_grid.bounding_box(ijk); };

		auto virtual operator()(math::Vector<f32, 3> const& pos) const -> T {
			return sampler(nanovdb_grid->worldToIndex(to_nanovdb(pos)));
		};
		auto virtual operator[](math::Vector<i32, 3> const& ijk) -> T& { return majorant_grid[ijk]; };
		auto virtual operator[](math::Vector<i32, 3> const& ijk) const -> T const& { return majorant_grid[ijk]; };

	private:
        nanovdb::GridHandle<> handle;
        nanovdb::Grid<nanovdb::NanoTree<T>> const* nanovdb_grid;
		Nanovdb_Sampler<T> sampler;
		math::Uniform_Grid<T, x, y, z> majorant_grid;
	};

    struct Nanovdb_Medium final: Medium {
		struct Cache final {
			math::Ray r{
				{math::maxv<f32>},
				{0.f}
			};
			math::Bounding_Box bbox{};
			f32 t_max{-1.f};
			f32 density_maj{0.f};
			spectra::Stochastic_Spectrum sigma_maj{};
			math::Exponential_Distribution distr{0.f};
		};

        Nanovdb_Medium(
            Nanovdb_Grid<f32, 64, 64, 64> const* grid,
            std::unique_ptr<spectra::Spectrum> sigma_a,
            std::unique_ptr<spectra::Spectrum> sigma_s,
            std::unique_ptr<spectra::Spectrum> Le,
			std::unique_ptr<phase::Phase_Function> phase,
            f32 density_scale = 1.0f
        );

		~Nanovdb_Medium();
        
        auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
        
    private:
        std::unique_ptr<spectra::Spectrum> sigma_a;
        std::unique_ptr<spectra::Spectrum> sigma_s;
        std::unique_ptr<spectra::Spectrum> Le;
		std::unique_ptr<phase::Phase_Function> phase;

		Nanovdb_Grid<f32, 64, 64, 64> const* grid;
        f32 density_scale;

		static thread_local std::unordered_map<Nanovdb_Medium const*, Cache> thread_caches;
    };
}
