#include <metatron/render/texture/image.hpp>
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/math/cmath.hpp>
#include <bit>

namespace mtt::render {

namespace {

struct Context final {
    ft2v texture;
    ftw2v weights;
    Image_Distribution distribution;
};

[[mtt::kernel]] auto weight(Context& ctx, uv3 px) noexcept -> void {
    auto uv = (fv2{px} + 0.5f) / fv2{ctx.weights.size()};
    auto c = ctx.texture[fv3{uv, 0.f}];
    auto w = core::abs(core::avg(core::shrink(c)));
    if (ctx.distribution == Image_Distribution::spherical) w *= core::sin(uv[1] * core::pi);
    ctx.weights[uzv3{px}] = fv4{w, 0.f, 0.f, 0.f};
}

}

Image_Vector_Texture::Image_Vector_Texture(
    std::string_view path, Image_Distribution distribution, bool linear, core::recorder& recorder
) noexcept {
    texture = ft2{};
    if (distribution == Image_Distribution::none) return;
    auto extent = this->texture.size();
    auto size = uzv2{std::bit_ceil(extent[0]), std::bit_ceil(extent[1])};
    auto weights = ftw2{};
    weights.update<core::ordinary<32>, core::encoding::sf>({size, 1, 1});
    auto ctx = Context{this->texture, weights, distribution};
    // core::pipenc{recorder, weight, ctx}.dispatch({uv2{size}, 1});
    distr.update(recorder, weights);
    recorder.retain(std::move(weights));
}

auto Image_Vector_Texture::operator()(
    fv2 const& uv, fv4 const& duvdxy
) const noexcept -> fv4 {
    return texture[uv, duvdxy];
}

auto Image_Vector_Texture::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> fv2 {
    return distr.sample(u);
}

auto Image_Vector_Texture::pdf(fv2 const& uv) const noexcept -> f32 {
    return distr.pdf(uv);
}

Image_Spectrum_Texture::Image_Spectrum_Texture(
    std::string_view path,
    Color_Space::Spectrum_Type type,
    Image_Distribution distribution,
    Color_Space const* color_space,
    core::recorder& recorder
) noexcept
: color_space(color_space), type(type), image_tex(path, distribution, false, recorder) {}

auto Image_Spectrum_Texture::operator()(fv2 const& uv, fv4 const& duvdxy, fv4 const& spec) const noexcept -> fv4 {
    auto rgba = image_tex(uv, duvdxy);
    auto rgb_spec = Rgb_Spectrum{
        rgba,
        type,
        color_space,
    };
    return spec & rgb_spec;
}

auto Image_Spectrum_Texture::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> fv2 {
    return image_tex.sample(ctx, u);
}

auto Image_Spectrum_Texture::pdf(fv2 const& uv) const noexcept -> f32 {
    return image_tex.pdf(uv);
}

}
