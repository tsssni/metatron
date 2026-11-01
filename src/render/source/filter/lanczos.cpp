#include <metatron/render/filter/lanczos.hpp>
#include <metatron/core/math/trigonometric.hpp>

namespace mtt::render {

namespace {

struct Context final {
    fv2 radius;
    f32 tau;
    ftw2v weights;
};

[[mtt::kernel]] auto weight(Context& ctx, uv3 px) noexcept -> void {
    auto p = (fv2{px} + 0.5f) / fv2{ctx.weights.size()} * (2.f * ctx.radius) - ctx.radius;
    auto w = core::windowed_sinc(p[0], ctx.tau) * core::windowed_sinc(p[1], ctx.tau);
    ctx.weights[uzv3{px}] = fv4{core::abs(w), 0.f, 0.f, 0.f};
}

}

Lanczos_Filter::Lanczos_Filter(fv2 radius, f32 tau, core::recorder& recorder) noexcept
: radius(radius), tau(tau) {
    auto weights = ftw2{};
    weights.update<core::ordinary<32>, core::encoding::sf>({64, 64, 1, 1});
    auto ctx = Context{radius, tau, weights};
    // core::pipenc{recorder, weight, ctx}.dispatch({64, 64, 1});
    distr.update(recorder, weights);
    recorder.retain(std::move(weights));
}

auto Lanczos_Filter::operator()(fv2 const& p) const noexcept -> f32 {
    return prod(foreach([&](f32 x, usize i) -> f32 { return core::abs(x) > radius[i] ? 0.f : core::windowed_sinc(x, tau); }, p));
}

auto Lanczos_Filter::sample(fv2 const& u) const noexcept -> Filter_Interaction {
    auto uv = distr.sample(u);
    auto p = uv * (2.f * radius) - radius;
    auto w = (*this)(p);
    auto pdf = distr.pdf(uv) / core::prod(2.f * radius);
    return {p, w, pdf};
}

}
