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
        auto post_args = make_desc<shader::Argument>({"metatron/render/monte-carlo/postprocess.constants"});
        auto postprocess = make_desc<shader::Pipeline>({"metatron/render/monte-carlo/postprocess.wave", {post_args.get()}});

        auto textures_args_encoder = encoder::Argument_Encoder{render.get(), res.textures.get()};
        auto images_view = resources.images
        | std::views::transform([](auto&& x) -> opaque::Image::View { return *x; })
        | std::ranges::to<std::vector<opaque::Image::View>>();
        if (!images_view.empty()) textures_args_encoder.bind("textures.bindless", {0, images_view});
        textures_args_encoder.bind("textures.sampler", sampler.get());
        textures_args_encoder.upload();
        textures_args_encoder.submit();

        auto grids_args_encoder = encoder::Argument_Encoder{render.get(), res.grids.get()};
        auto grids_view = resources.grids
        | std::views::transform([](auto&& x) -> opaque::Grid::View { return *x; })
        | std::ranges::to<std::vector<opaque::Grid::View>>();
        if (!grids_view.empty()) grids_args_encoder.bind("grids.bindless", {0, grids_view});
        grids_args_encoder.upload();
        grids_args_encoder.submit();

        auto postprocess_args_encoder = encoder::Argument_Encoder{render.get(), post_args.get()};
        postprocess_args_encoder.bind("constants.film", *film);
        postprocess_args_encoder.bind("constants.image", *image);
        postprocess_args_encoder.upload();
        postprocess_args_encoder.submit();

        auto next = 1u;
        auto spp = ctx.film->spp;
        auto range = uv2{0, 1};
        auto progress = stl::progress{spp};
        auto seed = std::random_device{}();
        stl::print("seed: 0x{:x}", seed);

        struct { uptr vectors; uptr volumes; uptr fresnel; } global{
            resources.vecarr->addr,
            resources.volarr ? resources.volarr->addr : 0,
            (uptr)bsdf::Physical_Bsdf::fresnel_reflectance_table.ptr,
        };

        auto resources_args_encoder = encoder::Argument_Encoder{render.get(), res.resources.get()};
        resources_args_encoder.bind("resources.accel", accel.get());
        resources_args_encoder.acquire("resources", global);
        resources_args_encoder.upload();
        resources_args_encoder.submit();

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
                auto args_encoder = encoder::Argument_Encoder{cmd.get(), post_args.get()};
                args_encoder.acquire("constants.film", *film);
                args_encoder.acquire("constants.image", *image);
                args_encoder.submit();
                auto render_encoder = encoder::Pipeline_Encoder{cmd.get(), postprocess.get()};
                render_encoder.bind();
                render_encoder.dispatch(threads, group);
                render_encoder.submit();
                auto transfer_encoder = encoder::Transfer_Encoder{cmd.get()};
                transfer_encoder.copy(*buffer, *image);
                transfer_encoder.submit();
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
                auto render_cmd = render_queue->allocate({
                    {render_timeline.get(), render_count},
                    {shared_timeline.get(), scheduled_count},
                });
                auto transfer_cmd = transfer_queue->allocate({
                    {render_timeline.get(), render_count + 1},
                    {network_timeline.get(), scheduled_count},
                });

                if (scheduled_count > 0) {
                    auto acquire_encoder = encoder::Transfer_Encoder{render_cmd.get()};
                    acquire_encoder.transfer(*image, render_queue.get(), transfer_queue.get());
                    acquire_encoder.submit();
                }

                auto args_encoder = encoder::Argument_Encoder{render_cmd.get(), post_args.get()};
                args_encoder.acquire("constants.film", *film);
                args_encoder.acquire("constants.image", *image);
                args_encoder.submit();

                auto render_encoder = encoder::Pipeline_Encoder{render_cmd.get(), postprocess.get()};
                render_encoder.bind();
                render_encoder.dispatch(threads, group);
                render_encoder.submit();

                auto release_encoder = encoder::Transfer_Encoder{render_cmd.get()};
                release_encoder.transfer(*image, transfer_queue.get(), render_queue.get());
                release_encoder.submit();

                auto transfer_encoder = encoder::Transfer_Encoder{transfer_cmd.get()};
                transfer_encoder.transfer(*image, transfer_queue.get(), render_queue.get());
                transfer_encoder.copy(*buffer, *image);
                transfer_encoder.transfer(*image, render_queue.get(), transfer_queue.get());
                transfer_encoder.submit();

                render_queue->submit(std::move(render_cmd), {{render_timeline.get(), ++render_count}});
                transfer_queue->submit(std::move(transfer_cmd), {{shared_timeline.get(), ++scheduled_count}});
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
