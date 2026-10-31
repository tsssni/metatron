#include <metatron/render/renderer/renderer.hpp>
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/resource/serde/args.hpp>
#include <metatron/core/stl/chrono.hpp>
#include <random>

namespace mtt::renderer {
    [[mtt::kernel]] auto normalize(muldim::Image& image, uv3 px) noexcept -> void {
        if (px[0] >= image.width || px[1] >= image.height) return;
        auto pixel = fv4{image[px[0], px[1]]};
        pixel /= pixel[3];
        image[px[0], px[1]] = pixel;
    }

    auto Renderer::trace() noexcept -> void {
        auto& args = scene::Args::instance();
        auto rd = std::random_device{};
        auto seed = rd();
        stl::print("seed: 0x{:x}", seed);

        for (auto i = 0u; i < stl::vector<void>::size(); ++i)
            stl::vector<void>::raw(i).pack();

        auto queue = infra::queue{};
        auto timeline = infra::timeline{};
        auto signal = 0ull;

        auto ctx = monte_carlo::Context{};
        auto spp = ctx.film->spp;
        auto size = uv2{ctx.film->image.size};

        auto progress = stl::progress{spp};
        for (auto i = 0u; i < spp; ++i) {
            auto recorder = infra::recorder{&queue, {{&timeline, signal}}};
            ctx.sample_index = i;
            ctx.seed = seed;
            ctx.integrator.trace(recorder, ctx);
            recorder.submit({{&timeline, ++signal}});
            ++progress;
        }

        {
            auto recorder = infra::recorder{&queue, {{&timeline, signal}}};
            infra::pipenc{recorder, normalize, ctx.film->image}.dispatch({size, 1});
            recorder.submit({{&timeline, ++signal}});
        }

        timeline.wait(signal);
        ctx.film->image.to_path(args.output, ctx.film->color_space);
        ~progress;
    }
}
