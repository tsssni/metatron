#include <metatron/render/renderer/renderer.hpp>
#include <metatron/resource/serde/args.hpp>
#include <metatron/network/remote/preview.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/chrono.hpp>

namespace mtt::renderer {
    auto Renderer::trace() noexcept -> void {
        auto& args = scene::Args::instance();
        auto rd = std::random_device{};
        auto seed = rd();
        stl::print("seed: 0x{:x}", seed);

        auto addr = wired::Address{args.address};
        for (auto i = 0u; i < stl::vector<void>::size(); ++i)
            stl::vector<void>::raw(i).pack();

        auto ctx = monte_carlo::Context{};
        auto spp = ctx.film->spp;
        auto size = uzv2{ctx.film->image.size};
        auto intg = monte_carlo::Integrator::entity("/integrator");
        intg.acquire(ctx, {});

        auto& film = ctx.film->image;
        auto image = muldim::Image{.size = film.size, .linear = film.linear};
        image.pixels.emplace_back(film.pixels.front().size());

        auto range = uv2{0, 1};
        auto progress = stl::progress{spp};

        auto next = 1u;
        auto previewer = remote::Previewer{addr, "metatron"};
        auto future = std::shared_future<void>{stl::scheduler::async_dispatch([]{})};

        while (range[0] < spp) {
            for (auto i = range[0]; i < range[1]; i++) {
                ctx.sample_index = i;
                ctx.seed = seed;
                intg.trace(ctx);
                ++progress;
            }

            stl::scheduler::sync_parallel(uzv2{size}, [&film, &image](cref<uzv2> px) {
                auto [i, j] = px;
                auto pixel = fv4{film[i, j]};
                pixel /= pixel[3];
                image[i, j] = pixel;
            });

            range[0] = range[1];
            range[1] = math::min(spp, range[1] + next);
            next = math::min(next * 2, ctx.film->stride);

            future.wait();
            if (range[0] == spp) image.to_path(args.output, ctx.film->color_space);
            if (!addr.host.empty()) future = stl::scheduler::async_dispatch(
                [&image, &previewer] { previewer.update(image); }
            );
        }

        future.wait();
        ~progress;
    }
}
