#include <metatron/core/math/gaussian.hpp>
#include <metatron/render/filter/gaussian.hpp>

namespace mtt::render {

namespace {

struct Context final {
    fv2 radius;
    f32 sigma;
    ftw2v weights;
};

[[mtt::kernel]] auto weight(Context& ctx, uv3 px) noexcept -> void {
    auto p = (fv2{px} + 0.5f) / fv2{ctx.weights.size()} * (2.f * ctx.radius) - ctx.radius;
    auto vx = core::gaussian(p[0], 0.f, ctx.sigma) - core::gaussian(ctx.radius[0], 0.f, ctx.sigma);
    auto vy = core::gaussian(p[1], 0.f, ctx.sigma) - core::gaussian(ctx.radius[1], 0.f, ctx.sigma);
    ctx.weights[uzv3{px}] = fv4{core::abs(vx * vy), 0.f, 0.f, 0.f};
}

}

Gaussian_Filter::Gaussian_Filter(fv2 radius, f32 sigma, core::recorder& recorder) noexcept
: radius(radius), sigma(sigma) {
    auto weights = ftw2{};
    weights.update<core::ordinary<32>, core::encoding::sf>({64, 64, 1, 1});
    auto ctx = Context{radius, sigma, weights};
    // core::pipenc{recorder, weight, ctx}.dispatch({64, 64, 1});
    distr.update(recorder, weights);
    recorder.retain(std::move(weights));
}

auto Gaussian_Filter::operator()(fv2 const& p) const noexcept -> f32 {
    auto vx = core::gaussian(p[0], 0.f, sigma) - core::gaussian(radius[0], 0.f, sigma);
    auto vy = core::gaussian(p[1], 0.f, sigma) - core::gaussian(radius[1], 0.f, sigma);
    return vx * vy;
}

auto Gaussian_Filter::sample(fv2 const& u) const noexcept -> render::Filter_Interaction {
    auto uv = distr.sample(u);
    auto p = uv * (2.f * radius) - radius;
    auto w = (*this)(p);
    auto pdf = distr.pdf(uv) / core::prod(2.f * radius);
    return {p, w, pdf};
}

}
