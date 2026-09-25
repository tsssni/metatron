#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/serde/serde.hpp>
#include <metatron/core/math/plane.hpp>

namespace glz {
    template<>
    struct meta<mtt::texture::Image_Distribution> {
        using enum mtt::texture::Image_Distribution;
        auto constexpr static value = glz::enumerate(none, uniform, spherical);
    };
}

namespace mtt::texture {
    auto Vector_Texture::init() noexcept -> void {
        MTT_DESERIALIZE(
            Constant_Vector_Texture,
            Image_Vector_Texture
        );
        stl::vector<muldim::Image>::init();
        stl::vector<math::Planar_Distribution>::init();
    }

    auto Spectrum_Texture::init() noexcept -> void {
        MTT_DESERIALIZE(
            Constant_Spectrum_Texture,
            Image_Spectrum_Texture,
            Checkerboard_Texture
        );

        [&]<typename... Ss>(stl::array<Ss...>*) {
            using svec = spectra::Spectrum::vs;
            auto add = [&]<typename S>() { for (auto const& path: svec::keys<S>())
            Spectrum_Texture::push<Constant_Spectrum_Texture>(
                std::string{path}.replace(1, 8, "texture"),
                {spectra::Spectrum{svec::entity<S>(path)}}
            ); };
            (add.template operator()<Ss>(), ...);
        }((spectra::Spectrum::ts*)nullptr);
    }

    auto grad(
        cref<math::Ray_Differential> diff,
        cref<shape::Interaction> intr
    ) noexcept -> opt<muldim::Coordinate> {
        auto tangent = math::Plane{intr.p, intr.n};
        MTT_OPT_OR_RETURN(dt, math::hit(diff.r, tangent), {});
        MTT_OPT_OR_RETURN(dxt, math::hit(diff.rx, tangent), {});
        MTT_OPT_OR_RETURN(dyt, math::hit(diff.ry, tangent), {});

        auto p = diff.r.o + dt * diff.r.d;
        auto dpdx = diff.rx.o + dxt * diff.rx.d - p;
        auto dpdy = diff.ry.o + dyt * diff.ry.d - p;
        auto dpduv = math::transpose(fm23{intr.dpdu, intr.dpdv});
        auto duvdx = math::least_squares(dpduv, dpdx);
        auto duvdy = math::least_squares(dpduv, dpdy);

        return muldim::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
    }

    auto propagate(
        cref<math::Ray_Differential> diff,
        cref<shape::Interaction> intr,
        cref<muldim::Coordinate> coord,
        cref<fv3> wi, cref<fv4> eta
    ) noexcept -> math::Ray_Differential {
        auto wo = -math::normalize(diff.r.d);
        auto reflective = math::dot(wo, intr.n) * math::dot(wi, intr.n) > 0.f;
        auto n = math::normalize(reflective ? wo + wi : wo + eta[0] * wi);
        n = math::dot(n, wo) < 0.f ? -n : n;

        auto p = intr.p;
        auto dpdx = intr.dpdu * coord.dudx + intr.dpdv * coord.dvdx;
        auto dpdy = intr.dpdu * coord.dudy + intr.dpdv * coord.dvdy;
        auto dndx = intr.dndu * coord.dudx + intr.dndv * coord.dvdx;
        auto dndy = intr.dndu * coord.dudy + intr.dndv * coord.dvdy;

        auto derive = [&](cref<math::Ray> r, cref<fv3> dpdx, cref<fv3> dndx) -> math::Ray {
            auto d = math::normalize(r.d);
            auto m = math::normalize(n + dndx);
            auto wx = reflective ? math::reflect(d, m) : math::refract(d, m, eta[0]);
            return {p + dpdx, math::normalize(wx)};
        };

        return {{p, wi}, derive(diff.rx, dpdx, dndx), derive(diff.ry, dpdy, dndy)};
    }
}
