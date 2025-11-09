#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <OpenImageIO/imageio.h>
#include <bit>

namespace mtt::texture {
    Image_Vector_Texture::Image_Vector_Texture(Descriptor const& desc) noexcept {
        MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(desc.path), {
            std::println("mesh {} not exists", desc.path);
            std::abort();
        });
        auto in = OIIO::ImageInput::open(path.c_str());
        if (!in) {
            std::println("cannot find image {}", desc.path);
            std::abort();
        }

        auto& spec = in->spec();
        auto tex = image::Image{};
        tex.linear = desc.linear;
        tex.size = {
            usize(spec.width),
            usize(spec.height),
            usize(spec.nchannels),
            spec.format.size(),
        };
        tex.pixels.resize(std::bit_width(std::max(tex.width, tex.height)));
        tex.pixels.front().resize(math::prod(tex.size));

        auto success = in->read_image(0, 0, 0, spec.nchannels, spec.format, tex.pixels.front().data());
        if (!success) {
            std::println("can not read image {}", desc.path);
            std::abort();
        }
        in->close();

        auto size = math::Vector<usize, 2>(tex.size);
        auto channels = tex.size[2];
        auto stride = tex.size[3];
        if (desc.distr == Image_Distribution::none) return;

        auto pdf = std::vector<f32>(math::prod(size));
        stl::scheduler::instance().sync_parallel(size, [&](auto px) mutable {
            auto c = math::Vector<f32, 4>{tex[px[0], px[1]]};
            auto w = 1.f;
            if (desc.distr == Image_Distribution::spherical) {
                auto v = (px[1] + 0.5f) / size[1];
                auto theta = v * math::pi;
                w = std::sin(theta);
            }
            pdf[px[0] + px[1] * size[0]] = math::avg(math::shrink(c)) * w;
        });
        distr = {std::span{pdf}, math::reverse(size), {0.f}, {1.f}};

        for (auto mip = 1uz; mip < tex.pixels.size(); ++mip) {
            size[0] = std::max(1uz, size[0] >> 1uz);
            size[1] = std::max(1uz, size[1] >> 1uz);
            tex.pixels[mip].resize(math::prod(size) * channels * stride);

            auto down = [this, mip, &tex](math::Vector<usize, 2> const& px) mutable {
                auto [i, j] = px;
                tex[i, j, mip] = 0.25f * (0.f
                + math::Vector<f32, 4>{tex[i * 2uz + 0, j * 2uz + 0, mip - 1]}
                + math::Vector<f32, 4>{tex[i * 2uz + 0, j * 2uz + 1, mip - 1]}
                + math::Vector<f32, 4>{tex[i * 2uz + 1, j * 2uz + 0, mip - 1]}
                + math::Vector<f32, 4>{tex[i * 2uz + 1, j * 2uz + 1, mip - 1]}
                );
            };
            if (math::prod(size) > 1024)
                stl::scheduler::instance().sync_parallel(size, down);
            else for (auto j = 0; j < size[1]; ++j)
                    for (auto i = 0; i < size[0]; ++i)
                        down({i, j});
        }

        auto& vec = stl::vector<image::Image>::instance();
        auto lock = vec.lock();
        texture = vec.push_back(std::move(tex));
    }

    auto Image_Vector_Texture::operator()(
        image::Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4> {
        return (*texture)(coord);
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
        image::Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum {
        auto rgba = image_tex(coord);
        std::println("{}", rgba);
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
