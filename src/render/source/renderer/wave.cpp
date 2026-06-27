#include "resource.hpp"
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/resource/serde/args.hpp>
#include <metatron/device/command/context.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>
#include <metatron/network/remote/preview.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/chrono.hpp>

namespace mtt::renderer {
    auto upload(
        mut<command::Queue> queue,
        ref<std::vector<obj<command::Timeline>>> timelines
    ) noexcept -> Resources;

    auto build(
        mut<command::Queue> queue,
        std::span<obj<command::Timeline>> uploads,
        mut<command::Timeline> timeline,
        ref<u64> count
    ) noexcept -> obj<opaque::Acceleration>;

    auto Renderer::wave() noexcept -> void {
        auto& args = scene::Args::instance();
        auto addr = wired::Address{args.address};
        auto remote = !addr.host.empty();
        auto previewer = remote::Previewer{addr, "metatron"};

        command::Context::init();
        auto render_queue = make_obj<command::Queue>(command::Type::render);
        auto transfer_queue = make_obj<command::Queue>(command::Type::transfer);

        auto intg = monte_carlo::Integrator::entity("/integrator");
        auto ctx = monte_carlo::Context{};
        auto res = monte_carlo::Resources{};

        auto upload_timelines = std::vector<obj<command::Timeline>>(stl::scheduler::size());
        auto render_timeline = make_obj<command::Timeline>(!remote);
        auto shared_timeline = make_obj<command::Timeline>(true);
        auto network_timeline = make_obj<command::Timeline>(true);
        auto render_count = u64{0};
        auto scheduled_count = u64{0};

        intg.upload(ctx);
        auto resources = upload(render_queue.get(), upload_timelines);
        auto accel = build(render_queue.get(), upload_timelines, render_timeline.get(), render_count);
        auto render = render_queue->allocate({{render_timeline.get(), render_count}});

        auto sampler = make_desc<opaque::Sampler>({opaque::Sampler::Mode::repeat});
        auto film = make_desc<opaque::Image>({&ctx.film->image, opaque::Image::State::storable, command::Type::render});
        auto image = make_desc<opaque::Image>({&ctx.film->image, opaque::Image::State::storable, command::Type::render});
        auto buffer = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::visible,
            .type = remote ? command::Type::transfer : command::Type::render,
            .size = math::prod(ctx.film->image.size),
        });

        res.resources = make_desc<shader::Argument>({"resources"});
        res.textures = make_desc<shader::Argument>({"textures"});
        res.grids = make_desc<shader::Argument>({"grids"});
        auto arguments = make_desc<shader::Argument>({"metatron/render/monte-carlo/postprocess.constants"});
        auto postprocess = make_desc<shader::Pipeline>({"metatron/render/monte-carlo/postprocess.wave", {arguments.get()}});

        struct {
            mut<opaque::Sampler> sampler;
            std::vector<opaque::Image::View> textures;
        } textures{
            sampler.get(),
            resources.images
            | std::views::transform([](auto&& x) -> opaque::Image::View { return *x; })
            | std::ranges::to<std::vector<opaque::Image::View>>(),
        }; {
            auto encoder = encoder::Argument_Encoder{render.get(), res.textures.get()};
            encoder.push(textures, {0, sizeof(textures)});
            encoder.push(textures, 0, textures.textures);
            encoder.submit();
        }

        struct {
            std::vector<opaque::Grid::View> grids;
        } grids {
            resources.grids
            | std::views::transform([](auto&& x) -> opaque::Grid::View { return *x; })
            | std::ranges::to<std::vector<opaque::Grid::View>>()
        }; {
            auto encoder = encoder::Argument_Encoder{render.get(), res.grids.get()};
            encoder.push(grids, 0, grids.grids);
            encoder.submit();
        }

        struct {
            opaque::Image::View film;
            opaque::Image::View image;
        } constants{*film, *image}; {
            auto encoder = encoder::Argument_Encoder{render.get(), arguments.get()};
            encoder.push(constants, {0, sizeof(constants)});
            encoder.submit();
        }

        auto next = 1u;
        auto spp = ctx.film->spp;
        auto range = uv2{0, 1};
        auto progress = stl::progress{spp};
        auto seed = std::random_device{}();
        stl::print("seed: 0x{:x}", seed);

        struct {
            uptr vectors; uptr volumes; uptr fresnel;
            mut<opaque::Acceleration> accel;
        } global{
            resources.vecarr->addr,
            resources.volarr ? resources.volarr->addr : 0,
            (uptr)bsdf::Physical_Bsdf::fresnel_reflectance_table.ptr,
            accel.get(),
        }; {
            auto encoder = encoder::Argument_Encoder{render.get(), res.resources.get()};
            encoder.push(global, {0, sizeof(global)});
            encoder.submit();
        }

        ctx.seed = seed;
        ctx.render = render.get();
        ctx.image = film.get();
        intg.acquire(ctx, res);
        render_queue->submit(std::move(render), {{render_timeline.get(), ++render_count}});

        auto threads = uv3{film->width, film->height, 1};
        auto group = uv3{8, 8, 1};
        auto future = stl::scheduler::async_dispatch(
        [&, count = remote ? 1 : render_count + 1] mutable {
            auto range = uv2{0, 1};
            auto next = 1u;

            while (range[0] < spp) {
                if (remote) {
                    shared_timeline->wait(count);
                    previewer.update(ctx.film->image, {buffer->ptr, buffer->size});
                    network_timeline->signal(count);
                } else render_timeline->wait(count);
                ++count; progress + (range[1] - range[0]);
                range = {range[1], range[1] + next};
                next = math::min(next * 2, ctx.film->stride);
            }

            if (!remote) {
                auto cmd = render_queue->allocate({});

                auto barrier = encoder::Transfer_Encoder{cmd.get()};
                barrier.liberate(*film);
                barrier.liberate(*image);
                barrier.submit();

                auto render = encoder::Pipeline_Encoder{cmd.get(), postprocess.get()};
                render.bind();
                render.dispatch(threads, group);
                render.submit();

                auto transfer = encoder::Transfer_Encoder{cmd.get()};
                transfer.copy(*buffer, *image);
                transfer.submit();

                render_queue->submit(std::move(cmd), {{render_timeline.get(), ++render_count}});
                render_timeline->wait(render_count);
            }
        });

        while (range[0] < spp) {
            auto cmd = render_queue->allocate({{render_timeline.get(), render_count}});
            ctx.render = cmd.get();
            for (auto i = range[0]; i < range[1]; i++) {
                ctx.sample_index = i;
                intg.wave(ctx);
            }
            render_queue->submit(std::move(cmd), {{render_timeline.get(), ++render_count}});

            if (remote) {
                auto cmd = render_queue->allocate({
                    {render_timeline.get(), render_count},
                    {shared_timeline.get(), scheduled_count},
                });
                auto cme = transfer_queue->allocate({
                    {render_timeline.get(), render_count + 1},
                    {network_timeline.get(), scheduled_count},
                });

                if (scheduled_count > 0) {
                    auto acquire = encoder::Transfer_Encoder{cmd.get()};
                    acquire.transfer(*image, render_queue.get(), transfer_queue.get());
                    acquire.submit();
                }

                auto barrier = encoder::Transfer_Encoder{cmd.get()};
                barrier.liberate(*film);
                barrier.liberate(*image);
                barrier.submit();

                auto resolve = encoder::Pipeline_Encoder{cmd.get(), postprocess.get()};
                resolve.bind();
                resolve.dispatch(threads, group);
                resolve.submit();

                auto release = encoder::Transfer_Encoder{cmd.get()};
                release.transfer(*image, transfer_queue.get(), render_queue.get());
                release.submit();

                auto copy = encoder::Transfer_Encoder{cme.get()};
                copy.transfer(*image, transfer_queue.get(), render_queue.get());
                copy.copy(*buffer, *image);
                copy.transfer(*image, render_queue.get(), transfer_queue.get());
                copy.submit();

                render_queue->submit(std::move(cmd), {{render_timeline.get(), ++render_count}});
                transfer_queue->submit(std::move(cme), {{shared_timeline.get(), ++scheduled_count}});
            }

            range[0] = range[1];
            range[1] = math::min(spp, range[1] + next);
            next = math::min(next * 2, ctx.film->stride);
        }

        future.wait();
        intg.release();
        auto data = std::span<byte const>{buffer->ptr, buffer->size};
        ctx.film->image.to_path(args.output, ctx.film->color_space, data);
    }
}
