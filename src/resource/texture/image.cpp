#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <OpenImageIO/imageio.h>
#include <bit>

namespace mtt::texture {
    Image_Vector_Texture::Image_Vector_Texture(Descriptor const& desc) noexcept {
        auto in = OIIO::ImageInput::open(std::string{desc.path});
        if (!in) {
            std::println("cannot find image {}", desc.path);
            std::abort();
        }

        auto& spec = in->spec();
        auto& vec = stl::poly_vector<device::Texture>::instance();
        texture = vec.push_back<device::Texture>({});
        texture->linear = desc.linear;
        texture->size = {
            usize(spec.width),
            usize(spec.height),
            usize(spec.nchannels),
            spec.format.size(),
        };
        texture->pixels.resize(std::bit_width(std::max(texture->width, texture->height)));
        texture->pixels.front().resize(math::prod(texture->size));

        auto success = in->read_image(0, 0, 0, spec.nchannels, spec.format, texture->pixels.front().data());
        if (!success) {
            std::println("can not read image {}", desc.path);
            std::abort();
        }
        in->close();

        auto size = math::Vector<usize, 2>(texture->size);
        auto channels = texture->size[2];
        auto stride = texture->size[3];
        if (desc.distr == Image_Distribution::none) return;

        auto pdf = std::vector<f32>(math::prod(size));
        stl::scheduler::instance().sync_parallel(size, [&](auto px) mutable {
            auto c = math::Vector<f32, 4>{(*texture.data())[px[0], px[1]]};
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

    auto Image_Vector_Texture::operator()(
        device::Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4> {
        return (*texture.data())(coord);
    }

    auto Image_Vector_Texture::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2> {
        return math::reverse(distr.sample(u));
    }

    auto Image_Vector_Texture::pdf(
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32 {
        return distr.pdf(math::reverse(uv));
    }

    Image_Spectrum_Texture::Image_Spectrum_Texture(
        Descriptor const& desc
    ) noexcept:
    image_tex({desc.path, desc.distr, false}),
    type(desc.type),
    color_space(desc.color_space) {}


    auto Image_Spectrum_Texture::operator()(
        device::Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum {
        auto rgba = image_tex(coord);
        auto rgb_spec = spectra::Rgb_Spectrum{{
            rgba,
            type,
            color_space,
        }};
        return spec & (&rgb_spec);
    }

    auto Image_Spectrum_Texture::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2> {
        return image_tex.sample(ctx, u);
    }

    auto Image_Spectrum_Texture::pdf(
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32 {
        return image_tex.pdf(uv);
    }
}
