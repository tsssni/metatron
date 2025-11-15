#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <OpenImageIO/imageio.h>
#include <bit>

namespace mtt::texture {
    Image_Vector_Texture::Image_Vector_Texture(cref<Descriptor> desc) noexcept {
        MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(desc.path), {
            std::println("image {} not exists", desc.path);
            std::abort();
        });
        auto in = OIIO::ImageInput::open(path.c_str());
        if (!in) {
            std::println("oiio error: cannot open image {}", desc.path);
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
        tex.pixels.resize(std::bit_width(math::max(tex.width, tex.height)));
        tex.pixels.front().resize(math::prod(tex.size));

        auto success = in->read_image(0, 0, 0, spec.nchannels, spec.format, tex.pixels.front().data());
        if (!success) {
            std::println("can not read image {}", desc.path);
            std::abort();
        }
        in->close();

        auto size = uzv2(tex.size);
        auto channels = tex.size[2];
        auto stride = tex.size[3];

        for (auto mip = 1uz; mip < tex.pixels.size(); ++mip) {
            auto fetch = [mip, size, &tex](cref<uzv2> src) {
                auto px = math::clamp(src, {0}, size - 1);
                return fv4{tex[px[0], px[1], mip - 1]};
            };
            size[0] = math::max(1uz, size[0] >> 1uz);
            size[1] = math::max(1uz, size[1] >> 1uz);
            tex.pixels[mip].resize(math::prod(size) * channels * stride);

            auto down = [fetch, mip, &tex](cref<uzv2> px) mutable {
                auto [i, j] = px;
                tex[i, j, mip] = 0.25f * (0.f
                + fetch({i * 2uz + 0, j * 2uz + 0})
                + fetch({i * 2uz + 0, j * 2uz + 1})
                + fetch({i * 2uz + 1, j * 2uz + 0})
                + fetch({i * 2uz + 1, j * 2uz + 1})
                );
            };

            if (math::prod(size) > 1024)
                stl::scheduler::instance().sync_parallel(size, down);
            else for (auto j = 0; j < size[1]; ++j)
                    for (auto i = 0; i < size[0]; ++i)
                        down({i, j});
        }

        if (desc.distr != Image_Distribution::none) {
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

        auto& vec = stl::vector<image::Image>::instance();
        auto lock = vec.lock();
        texture = vec.push_back(std::move(tex));
    }

    auto Image_Vector_Texture::operator()(
        cref<image::Coordinate> coord
    ) const noexcept -> fv4 {
        return (*texture)(coord);
    }

    auto Image_Vector_Texture::sample(
        cref<eval::Context> ctx, cref<fv2> u
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
        cref<image::Coordinate> coord, cref<fv4> spec
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
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2 {
        return image_tex.sample(ctx, u);
    }

    auto Image_Spectrum_Texture::pdf(cref<fv2> uv) const noexcept -> f32 {
        return image_tex.pdf(uv);
    }
}
