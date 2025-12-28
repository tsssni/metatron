#include <metatron/render/sampler/proxy.hpp>

namespace mtt::sampler {
    auto Proxy_Sampler::start() noexcept -> void {
        sampler->start(ctx);
    }

    auto Proxy_Sampler::generate_1d() noexcept -> f32 {
        return sampler->generate_1d(ctx);
    }

    auto Proxy_Sampler::generate_2d() noexcept -> fv2 {
        return sampler->generate_2d(ctx);
    }

    auto Proxy_Sampler::generate_pixel_2d() noexcept -> fv2 {
        return sampler->generate_pixel_2d(ctx);
    }
}
