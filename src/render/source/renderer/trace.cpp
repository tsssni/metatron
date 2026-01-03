#include "renderer.hpp"
#include <metatron/render/scene/args.hpp>
#include <metatron/network/remote/preview.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/progress.hpp>
#include <metatron/core/stl/memory.hpp>
#include <metatron/core/stl/print.hpp>
#include <random>

namespace mtt::renderer {
    auto Renderer::Impl::trace() noexcept -> void {
        auto rd = std::random_device{};
        auto seed = rd();
        stl::print("seed: 0x{:x}", seed);

        auto& args = scene::Args::instance();
        auto addr = wired::Address{args.address};

        auto ct = *entity<math::Transform>("/hierarchy/camera/render");
        auto spp = desc.film->spp;
        auto depth = desc.film->depth;
        auto size = uzv2{desc.film->image.size};

        auto range = uv2{0, 1};
        auto progress = stl::progress{math::prod(size) * spp};
        auto trace = [&](cref<uzv2> px) {
            for (auto n = range[0]; n < range[1]; ++n) {
                auto sp = sampler::Proxy_Sampler{desc.sampler, {{}, px, size, n, spp, 0, seed}};
                sp.start();
                auto fixel = (*desc.film)(desc.filter.data(), px, sp.generate_pixel_2d());
                auto spec = spectra::Stochastic_Spectrum{sp.generate_1d()};
                MTT_OPT_OR_CALLBACK(s, photo::Camera{}.sample(
                    desc.lens.data(), fixel.position, fixel.dxdy, sp.generate_2d()
                ), stl::abort("ray generation failed"););
                s.ray_differential = ct ^ s.ray_differential;
                s.default_differential = ct ^ s.default_differential;

                auto ctx = monte_carlo::Context{
                    desc.accel.data(),
                    desc.emitter.data(),
                    &sp, spec.lambda,
                    s.ray_differential,
                    s.default_differential,
                    ct, px, n, depth,
                };
                MTT_OPT_OR_CALLBACK(Li, desc.integrator->sample(ctx),
                    stl::abort("invalid value appears in pixel {} sample {}", px, n);
                );
                Li.value /= s.pdf;
                fixel = Li;
                ++progress;
            }
        };

        auto& film = desc.film->image;
        auto image = muldim::Image{.size = film.size, .linear = film.linear};
        image.pixels.emplace_back(film.pixels.front().size());

        auto next = 1u;
        auto previewer = remote::Previewer{addr, "metatron"};
        auto& scheduler = stl::scheduler::instance();
        auto future = std::shared_future<void>{scheduler.async_dispatch([]{})};

        while (range[0] < spp) {
            stl::scheduler::instance().sync_parallel(uzv2{size}, trace);
            range[0] = range[1];
            range[1] = math::min(spp, range[1] + next);
            next = math::min(next * 2, desc.film->stride);

            auto finished = range[0] == spp;
            future.wait();
            scheduler.sync_parallel(
                uzv2{image.size},
                [&image, &film](auto const& px) {
                    auto [i, j] = px;
                    auto pixel = fv4{film[i, j]};
                    pixel /= pixel[3];
                    image[i, j] = pixel;
                }
            );
            if (finished) image.to_path(args.output, desc.film->color_space);
            if (!addr.host.empty()) future = scheduler.async_dispatch(
                [&image, &previewer] { previewer.update(image); }
            );
        }

        future.wait();
        ~progress;
    }
}
