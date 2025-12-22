implementing texture;

namespace mtt::texture {
    enum Image_Distribution {
        none,
        uniform,
        spherical,
    }

    struct Image_Spectrum_Texture: Spectrum_Texture {
        tag<spectra::Color_Space> color_space;
        spectra::Color_Space::Spectrum_Type type;
        Image_Vector_Texture image_tex;

        fv4 operator()(Coordinate coord, fv4 spec) {
            let rgb = image_tex(coord).rgb;
            let rgb_spec = spectra::Rgb_Spectrum(
                rgb,
                type,
                color_space,
            );
            return spec & rgb_spec;
        }

        func sample(math::Context ctx, fv2 u) -> fv2 {
            return image_tex.sample(ctx, u);
        }

        func pdf(fv2 uv) -> f32 {
            return image_tex.pdf(uv);
        }
    }

    struct Image_Vector_Texture: Vector_Texture {
        tag<u32> idx;
        math::Planar_Distribution distr;

        fv4 operator()(Coordinate coord) {
            let texture = global.textures[idx.index()];
            return texture.SampleGrad(
                global.sampler, coord.uv,
                {coord.dudx, coord.dvdx},
                {coord.dudy, coord.dvdy}
            );
        }

        func sample(math::Context ctx, fv2 u) -> fv2 {
            let vu = distr.sample(u);
            return vu.yx;
        }

        func pdf(fv2 uv) -> f32 {
            return distr.pdf(uv.yx);
        }
    }
}
