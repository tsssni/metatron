#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::texture {
    Image_Vector_Texture::Image_Vector_Texture(
        poly<image::Image> image
    ) noexcept {
        auto size = math::Vector<usize, 2>{image->size};
        auto channels = image->size[2];
        auto stride = image->size[3];

        auto max_mips = std::bit_width(math::max(size));
        auto pdf_mip = -1;
        images.reserve(max_mips);
        images.push_back(std::move(image));

        for (auto mip = 1uz; mip < max_mips; mip++) {
            size[0] = std::max(1uz, size[0] >> 1uz);
            size[1] = std::max(1uz, size[1] >> 1uz);
            if (pdf_mip == -1 && size[0] <= pdf_dim) {
                pdf_mip = mip;
            }

            images.push_back(make_poly<image::Image>(
                math::Vector<usize, 4>{size, channels, stride},
                images.front()->color_space,
                images.front()->linear
            ));
            auto& image = *images[mip];
            auto& upper = *images[mip - 1];

            stl::scheduler::instance().sync_parallel(size, [&image, &upper](auto px) {
                auto [i, j] = px;
                image[i, j] = 0.25f * (0.f
                 + math::Vector<f32, 4>{upper[i * 2uz + 0, j * 2uz + 0]}
                 + math::Vector<f32, 4>{upper[i * 2uz + 0, j * 2uz + 1]}
                 + math::Vector<f32, 4>{upper[i * 2uz + 1, j * 2uz + 0]}
                 + math::Vector<f32, 4>{upper[i * 2uz + 1, j * 2uz + 1]}
                );
            });
            std::atomic_thread_fence(std::memory_order_release);
        }

        auto pdf = math::Matrix<f32, pdf_dim / 2, pdf_dim>{};
        auto pdf_size = math::Vector<usize, 2>{pdf_dim / 2, pdf_dim};
        auto& pdf_image = *images[pdf_mip];

        stl::scheduler::instance().sync_parallel(pdf_size, [this, &pdf, pdf_mip, pdf_size](auto px) {
            auto uv = (math::Vector<f32, 2>{px} + 0.5f) / pdf_size;
            auto spec = images[pdf_mip]->color_space->to_spectrum(
                linear({uv}, pdf_mip, {}),
                color::Color_Space::Spectrum_Type::illuminant
            );
            pdf[px[0]][px[1]] = spectra::Spectrum::spectra["CIE-Y"] | spec;
        });
        std::atomic_thread_fence(std::memory_order_release);
        distr = decltype(distr){std::move(pdf), {0.f}, {1.f}};
    }

    auto Image_Vector_Texture::operator()(
        Sampler const& sampler,
        Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4> {
        auto du = math::Vector<f32, 2>{coord.dudx, coord.dudy};
        auto dv = math::Vector<f32, 2>{coord.dvdx, coord.dvdy};
        auto ul = math::length(du);
        auto vl = math::length(dv);
        auto sl = std::min(ul, vl);
        auto ll = std::max(ul, vl);

        auto c = coord;

        if (sl * sampler.anisotropy < ll && sl > 0.f) {
            auto s = ll / (sl * sampler.anisotropy);
            sl *= s;
            if (ul == sl) {
                c.dudx *= s;
                c.dudy *= s;
            } else {
                c.dvdx *= s;
                c.dvdy *= s;
            }
        } else if (sl == 0.f) {
            return nearest(coord, sampler.min_lod + sampler.lod_bias, sampler);
        }

        auto l = std::max(ul * images[0]->size[0], vl * images[0]->size[1]);
        auto f = l > 1.f ? sampler.min_filter : sampler.mag_filter;
        auto filter = f == Sampler::Filter::nearest
        ? &Image_Vector_Texture::nearest
        : &Image_Vector_Texture::linear;

        auto lod = std::min(sampler.max_lod, std::max(sampler.min_lod,
            images.size() - 1.f + std::log2(sl) + sampler.lod_bias
        ));
        if (sampler.mip_filter == Sampler::Filter::none) {
            return (this->*filter)(c, 0, sampler);
        } else if (sampler.mip_filter == Sampler::Filter::nearest) {
            auto lodi = std::min(images.size() - 1, usize(std::round(lod)));
            return (this->*filter)(c, lodi, sampler);
        } else {
            auto lodi = std::min(images.size() - 2, usize(lod));
            return math::lerp(
                (this->*filter)(c, lodi, sampler),
                (this->*filter)(c, lodi + 1, sampler),
                lod - lodi
            );
        }
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

    auto Image_Vector_Texture::fetch(
        math::Vector<i32, 3> texel, Sampler const& sampler
    ) const noexcept -> math::Vector<f32, 4> {
        auto px = math::Vector<i32, 2>{texel};
        auto lod = texel[2];
        auto size = math::Vector<i32, 2>{images[lod]->size};

        auto wrap = [](i32 x, i32 s, Sampler::Wrap w) -> i32 {
            auto c = std::clamp(x, 0, s - 1);
            if (c == x || w == Sampler::Wrap::edge) {
                return c;
            } else if (w == Sampler::Wrap::repeat) {
                return math::pmod(x, s);
            } else if (w == Sampler::Wrap::mirror) {
                auto b = math::pmod(x / s, 2);
                auto p = math::pmod(x, s);
                return b == 0 ? p : s - 1 - p;
            } else {
                return s;
            }
        };

        auto i = wrap(px[0], size[0], sampler.wrap_u);
        auto j = wrap(px[1], size[1], sampler.wrap_v);
        if (i == size[0] || j == size[1]) {
            return sampler.border;
        } else {
            return math::Vector<f32, 4>{(*images[lod])[i, j]};
        }
    }

    auto Image_Vector_Texture::nearest(
        Coordinate const& coord, i32 lod, Sampler const& sampler
    ) const noexcept -> math::Vector<f32, 4> {
        auto& image = *images[lod];
        auto uv = math::round(coord.uv * math::Vector<usize, 2>{image.size} - 0.5f);
        return fetch({uv, lod}, sampler);
    }

    auto Image_Vector_Texture::linear(
        Coordinate const& coord, i32 lod, Sampler const& sampler
    ) const noexcept -> math::Vector<f32, 4> {
        auto& image = *images[lod];
        auto uv = coord.uv * math::Vector<usize, 2>{image.size} - 0.5f;
        auto ux = coord.dudx * image.width;
        auto uy = coord.dudy * image.height;
        auto vx = coord.dvdx * image.width;
        auto vy = coord.dvdy * image.height;

        auto A = uy * uy + vy * vy + 1.f;
        auto B = -2.f * (ux * uy + vx * vy);
        auto C = ux * ux + vx * vx + 1.f;
        auto F = A * C -  B * B / 4.f;
        A = math::guarded_div(A, F);
        B = math::guarded_div(B, F);
        C = math::guarded_div(C, F);

        auto det = -B * B + 4.f * A * C;
        auto u_tan = 2.f * math::guarded_div(math::sqrt(det * C), det);
        auto v_tan = 2.f * math::guarded_div(math::sqrt(det * A), det);
        auto u_range = math::Vector<i32, 2>{
            i32(std::ceil(uv[0] - u_tan)),
            i32(std::ceil(uv[0] + u_tan)),
        };
        auto v_range = math::Vector<i32, 2>{
            i32(std::ceil(uv[1] - v_tan)),
            i32(std::ceil(uv[1] + v_tan)),
        };

        auto sum_t = math::Vector<f32, 4>{0.f};
        auto sum_w = 0.f;
        for (auto j = v_range[0]; j <= v_range[1]; j++) {
            auto ud = j - uv[1];
            for (auto i = u_range[0]; i <= u_range[1]; i++) {
                auto vd = i - uv[0];
                auto r2 = A * ud * ud + B * ud * vd + C * vd * vd;
                if (r2 < 1) {
                    // e^{-2r^2}-e^{-2}
                    auto constexpr ewa_lut = std::to_array({
                        0.864664733f,  0.849040031f,   0.83365953f,   0.818519294f,
                        0.80361563f,   0.788944781f,   0.774503231f,  0.760287285f,
                        0.746293485f,  0.732518315f,   0.718958378f,  0.705610275f,
                        0.692470789f,  0.679536581f,   0.666804492f,  0.654271305f,
                        0.641933978f,  0.629789352f,   0.617834508f,  0.606066525f,
                        0.594482362f,  0.583079159f,   0.571854174f,  0.560804546f,
                        0.549927592f,  0.539220572f,   0.528680861f,  0.518305838f,
                        0.50809288f,   0.498039544f,   0.488143265f,  0.478401601f,
                        0.468812168f,  0.45937258f,    0.450080454f,  0.440933526f,
                        0.431929469f,  0.423066139f,   0.414341331f,  0.405752778f,
                        0.397298455f,  0.388976216f,   0.380784035f,  0.372719884f,
                        0.364781618f,  0.356967449f,   0.34927541f,   0.341703475f,
                        0.334249914f,  0.32691282f,    0.319690347f,  0.312580705f,
                        0.305582166f,  0.298692942f,   0.291911423f,  0.285235822f,
                        0.278664529f,  0.272195935f,   0.265828371f,  0.259560347f,
                        0.253390193f,  0.247316495f,   0.241337672f,  0.235452279f,
                        0.229658857f,  0.223955944f,   0.21834214f,   0.212816045f,
                        0.207376286f,  0.202021524f,   0.196750447f,  0.191561714f,
                        0.186454013f,  0.181426153f,   0.176476851f,  0.171604887f,
                        0.166809067f,  0.162088141f,   0.157441005f,  0.152866468f,
                        0.148363426f,  0.143930718f,   0.139567271f,  0.135272011f,
                        0.131043866f,  0.126881793f,   0.122784719f,  0.11875169f,
                        0.114781633f,  0.11087364f,    0.107026696f,  0.103239879f,
                        0.0995122194f, 0.0958427936f,  0.0922307223f, 0.0886750817f,
                        0.0851749927f, 0.0817295909f,  0.0783380121f, 0.0749994367f,
                        0.0717130303f, 0.0684779733f,  0.0652934611f, 0.0621587038f,
                        0.0590728968f, 0.0560353249f,  0.0530452281f, 0.0501018465f,
                        0.0472044498f, 0.0443523228f,  0.0415447652f, 0.0387810767f,
                        0.0360605568f, 0.0333825648f,  0.0307464004f, 0.0281514227f,
                        0.0255970061f, 0.0230824798f,  0.0206072628f, 0.0181707144f,
                        0.0157722086f, 0.013411209f,   0.0110870898f, 0.0087992847f,
                        0.0065472275f, 0.00433036685f, 0.0021481365f, 0.f,
                    });

                    auto idx = std::min<usize>(r2 * ewa_lut.size(), ewa_lut.size() - 1);
                    auto w = ewa_lut[idx];
                    sum_t += w * fetch({i, j, lod}, sampler);
                    sum_w += w;
                }
            }
        }

        return sum_t / sum_w;
    }

    Image_Spectrum_Texture::Image_Spectrum_Texture(
        poly<image::Image> image,
        color::Color_Space::Spectrum_Type type
    ) noexcept: image_tex(std::move(image)), type(type) {}


    auto Image_Spectrum_Texture::operator()(
        Sampler const& sampler,
        Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum {
        auto rgba = image_tex(sampler, coord);
        auto rgb_spec = image_tex.images.front()->color_space->to_spectrum(rgba, type);
        return spec & rgb_spec;
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
