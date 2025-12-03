#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::texture {
    Image_Vector_Texture::Image_Vector_Texture(cref<Descriptor> desc) noexcept {
        auto tex = muldim::Image::from_path(desc.path, desc.linear);

        if (desc.distr != Image_Distribution::none) {
            auto size = uzv2{tex.size};
            auto pdf = std::vector<f32>(math::prod(size));
            stl::scheduler::instance().sync_parallel(size, [&](auto px) mutable {
                auto c = fv4{tex[px[0], px[1]]};
                auto w = 1.f;
                if (desc.distr == Image_Distribution::spherical) {
                    auto v = (px[1] + 0.5f) / size[1];
                    auto theta = v * math::pi;
                    w = std::sin(theta);
                }
                pdf[px[0] + px[1] * size[0]] = math::avg(math::shrink(c)) * w;
            });
            distr = {std::span{pdf}, math::reverse(size), {0.f}, {1.f}};
        }

        auto& vec = stl::vector<muldim::Image>::instance();
        auto lock = vec.lock();
        texture = vec.push_back(std::move(tex));
    }

    auto Image_Vector_Texture::operator()(
        cref<muldim::Coordinate> coord
    ) const noexcept -> fv4 {
        return (*texture)(coord);
    }

    auto Image_Vector_Texture::sample(
        cref<math::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2 {
        return math::reverse(distr.sample(u));
    }

    auto Image_Vector_Texture::pdf(cref<fv2> uv) const noexcept -> f32 {
        return distr.pdf(math::reverse(uv));
    }

    Image_Spectrum_Texture::Image_Spectrum_Texture(
        cref<Descriptor> desc
    ) noexcept:
    image_tex({desc.path, desc.distr, false}),
    type(desc.type),
    color_space(desc.color_space) {}


    auto Image_Spectrum_Texture::operator()(
        cref<muldim::Coordinate> coord, cref<fv4> spec
    ) const noexcept -> fv4 {
        auto rgba = image_tex(coord);
        auto rgb_spec = spectra::Rgb_Spectrum{{
            rgba,
            type,
            color_space,
        }};
        return spec & (&rgb_spec);
    }

    auto Image_Spectrum_Texture::sample(
        cref<math::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2 {
        return image_tex.sample(ctx, u);
    }

    auto Image_Spectrum_Texture::pdf(cref<fv2> uv) const noexcept -> f32 {
        return image_tex.pdf(uv);
    }
}
