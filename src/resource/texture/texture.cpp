#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/shape/plane.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::texture {
	auto grad(
		math::Ray_Differential const& diff,
		shape::Interaction const& intr
	) -> std::optional<texture::Coordinate> {
		auto tangent = shape::Plane{intr.p, intr.n};
		#define METATRON_DSTOP {terminated = true; gamma = 0.f; std::printf("\n===\n"); continue;}
		METATRON_OPT_OR_RETURN(d_intr, tangent(diff.r), {});
		METATRON_OPT_OR_RETURN(dx_intr, tangent(diff.rx), {});
		METATRON_OPT_OR_RETURN(dy_intr, tangent(diff.ry), {});
		
		auto dpdx = dx_intr.p - d_intr.p;
		auto dpdy = dy_intr.p - d_intr.p;
		auto dpduv =  math::transpose(math::Matrix<f32, 2, 3>{intr.dpdu, intr.dpdv});
		auto duvdx = math::least_squares(dpduv, dpdx);
		auto duvdy = math::least_squares(dpduv, dpdy);

		return texture::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
	}
}
