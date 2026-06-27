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
}
