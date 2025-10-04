#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/math/plane.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::texture {
    auto grad(
        math::Ray_Differential const& diff,
        shape::Interaction const& intr
    ) noexcept -> std::optional<texture::Coordinate> {
        auto tangent = math::Plane{intr.p, intr.n};
        MTT_OPT_OR_RETURN(dt, math::hit(diff.r, tangent), {});
        MTT_OPT_OR_RETURN(dxt, math::hit(diff.rx, tangent), {});
        MTT_OPT_OR_RETURN(dyt, math::hit(diff.ry, tangent), {});
        
        auto p = diff.r.o + dt * diff.r.d;
        auto dpdx = diff.rx.o + dxt * diff.rx.d - p;
        auto dpdy = diff.ry.o + dyt * diff.ry.d - p;
        auto dpduv =  math::transpose(math::Matrix<f32, 2, 3>{intr.dpdu, intr.dpdv});
        auto duvdx = math::least_squares(dpduv, dpdx);
        auto duvdy = math::least_squares(dpduv, dpdy);

        return texture::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
    }
}
