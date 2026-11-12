#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/core/stl/chrono.hpp>
#include <random>

namespace mtt::render {

[[mtt::kernel]] auto clear(frw2v& image, uv3 px) noexcept -> void {
    image[uzv3{px}] = fv4{0.f};
}

[[mtt::kernel]] auto normalize(frw2v& image, uv3 px) noexcept -> void {
    auto pixel = image[uzv3{px}];
    pixel /= pixel[3];
    image[uzv3{px}] = pixel;
}

auto Renderer::trace(Monte_Carlo_Context& ctx) noexcept -> void {
    auto rd = std::random_device{};
    auto seed = rd();
    core::print("seed: 0x{:x}", seed);

    auto queue = core::queue{};
    auto timeline = core::timeline{};
    auto signal = 0ull;

    auto spp = ctx.film->spp;
    auto size = uv2{ctx.film->image.size()};
    auto image = frw2v{ctx.film->image};

    {
        auto recorder = core::recorder{&queue, {}};
        core::pipenc{recorder, clear, image}.dispatch({size, 1});
        recorder.submit({{&timeline, ++signal}});
    }

    auto progress = core::progress{spp};
    for (auto i = 0u; i < spp; ++i) {
        auto recorder = core::recorder{&queue, {{&timeline, signal}}};
        ctx.sample_index = i;
        ctx.seed = seed;
        ctx.integrator.trace(recorder, ctx);
        recorder.submit({{&timeline, ++signal}});
        ++progress;
    }

    {
        auto recorder = core::recorder{&queue, {{&timeline, signal}}};
        core::pipenc{recorder, normalize, image}.dispatch({size, 1});
        recorder.submit({{&timeline, ++signal}});
    }

    timeline.wait(signal);
    ~progress;
}

}
