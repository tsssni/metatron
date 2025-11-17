#include <metatron/resource/image/image.hpp>
#include <metatron/core/stl/print.hpp>
#include <functional>

namespace mtt::image {
    auto sRGB_linearize(f32 x) noexcept -> f32 {
        if (x <= 0.04045f) return x / 12.92f;
        else return std::pow((x + 0.055f) / 1.055f, 2.4f);
    };

    auto sRGB_transfer(f32 x) noexcept -> f32 {
        if (x <= 0.0031308f) return 12.92f * x;
        else return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
    }

    Image::Pixel::Pixel(view<Image> image, mut<byte> start) noexcept
    : image(image), start(start) {}

    Image::Pixel::operator fv4() const noexcept {
        auto pixel = fv4{};
        for (auto i = 0; i < image->channels; ++i) {
            switch (image->stride) {
                case 1:
                    pixel[i] = image->linear
                    ? *(start + i) / 255.f
                    : sRGB_linearize(*(start + i) / 255.f);
                    break;
                case 4:
                    pixel[i] = *(mut<f32>(start) + i);
                    break;
                default:
                    break;
            }
        }
        return pixel;
    }

    auto Image::Pixel::operator=(cref<fv4> v) noexcept -> void {
        for (auto i = 0; i < image->size[2]; ++i) {
            auto* pixel = start + image->size[3] * i; 
            switch (image->size[3]) {
                case 1:
                    *pixel = image->linear
                        ? byte(v[i] * 255.f)
                        : byte(sRGB_transfer(v[i]) * 255.f);
                    break;
                case 4:
                    *mut<f32>(pixel) = v[i];
                    break;
                default:
                    break;
            }
        }
    }

    auto Image::Pixel::operator+=(cref<fv4> v) noexcept -> void {
        *this = fv4(*this) + v;
    }

    auto Image::Pixel::data() noexcept -> mut<byte> {
        return start;
    }

    auto Image::operator[](usize x, usize y, usize lod) noexcept -> Pixel {
        auto width = (pixels.size() == 1 || pixels[0].size() == pixels[1].size())
        ? this->width : (this->width >> lod);
        auto offset = (y * width + x) * channels * stride;
        return Pixel{this, &pixels[lod][offset]};
    }

    auto Image::operator[](usize x, usize y, usize lod) const noexcept -> Pixel const {
        return (*const_cast<mut<Image>>(this))[x, y, lod];
    }

    auto Image::operator()(cref<Coordinate> coord) const -> fv4 {
        auto du = fv2{coord.dudx, coord.dudy};
        auto dv = fv2{coord.dvdx, coord.dvdy};
        auto ul = math::length(du);
        auto vl = math::length(dv);
        auto sl = math::min(ul, vl);
        auto ll = math::max(ul, vl);

        auto constexpr anisotropy = 16.f;
        auto c = coord;
        if (sl * anisotropy < ll && sl > 0.f) {
            auto s = ll / (sl * anisotropy);
            sl *= s;
            if (ul == sl) {
                c.dudx *= s;
                c.dudy *= s;
            } else {
                c.dvdx *= s;
                c.dvdy *= s;
            }
        } else if (sl == 0.f) {
            auto [x, y] = iv2{math::round(coord.uv * fv2{width, height} - 0.5f)};
            x = math::pmod(x, i32(width));
            y = math::pmod(y, i32(height));
            return fv4{(*this)[x, y]};
        }

        auto lodm = pixels.size() - 1.f;
        auto lod = math::clamp(lodm + std::log2(sl), 0.f, lodm);

        auto filter = [&](i32 lod) -> fv4 {
            auto width = math::max(1uz, this->width >> lod);
            auto height = math::max(1uz, this->height >> lod);
            auto& pixels = this->pixels[lod];
            auto uv = coord.uv * uzv2{width, height} - 0.5f;
            auto ux = coord.dudx * width;
            auto uy = coord.dudy * height;
            auto vx = coord.dvdx * width;
            auto vy = coord.dvdy * height;

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
            auto u_range = iv2{
                i32(std::ceil(uv[0] - u_tan)),
                i32(std::ceil(uv[0] + u_tan)),
            };
            auto v_range = iv2{
                i32(std::ceil(uv[1] - v_tan)),
                i32(std::ceil(uv[1] + v_tan)),
            };

            auto sum_t = fv4{0.f};
            auto sum_w = 0.f;
            for (auto j = v_range[0]; j <= v_range[1]; ++j) {
                auto ud = j - uv[1];
                for (auto i = u_range[0]; i <= u_range[1]; ++i) {
                    auto vd = i - uv[0];
                    auto r2 = A * ud * ud + B * ud * vd + C * vd * vd;
                    if (r2 >= 1) continue;

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

                    auto idx = math::min<usize>(r2 * ewa_lut.size(), ewa_lut.size() - 1);
                    auto w = ewa_lut[idx];
                    auto wi = math::pmod(i, i32(width));
                    auto wj = math::pmod(j, i32(height));
                    sum_t += w * fv4{(*this)[wi, wj, lod]};
                    sum_w += w;
                }
            }
            return sum_t / sum_w;
        };

        auto lodi = math::min(i32(lod), math::max(0, i32(pixels.size()) - 2));
        return pixels.size() == 1 ? filter(lodi)
        : math::lerp(filter(lodi), filter(lodi + 1), lod - lodi);
    }

    auto Image::operator()(cref<fv3> uvw) const -> fv4 {
        auto pixel = uvw * fv3{width, height, pixels.size()};
        auto base = math::clamp(
            math::floor(pixel - 0.5f),
            fv3{0.f},
            fv3{width - 2, height - 2, pixels.size() - 2}
        );
        auto frac = math::clamp(
            pixel - 0.5f - base,
            fv3{0.f},
            fv3{1.f}
        );

        auto weights = std::array<std::function<auto(f32) -> f32>, 2>{
            [] (f32 x) { return 1.f - x; },
            [] (f32 x) { return x; },
        };
        auto offsets = iv2{0, 1};

        auto r = fv4{0.f};
        for (auto i = 0; i < 4; i++) {
            auto b0 = i & 1;
            auto b1 = (i & 2) >> 1;
            auto w = weights[b0](frac[0]) * weights[b1](frac[1]);
            auto o = base + uzv2{offsets[b0], offsets[b1]};

            if (base[2] + 1 >= pixels.size())
                r += w * fv4{(*this)[o[0], o[1], base[2]]};
            else
                r += w * math::lerp(
                    fv4{(*this)[o[0], o[1], base[2]]},
                    fv4{(*this)[o[0], o[1], base[2] + 1]},
                    frac[2]
                );
        }
        return r;
    }
}
