#include "renderer.hpp"
#include "resource.hpp"
#include <metatron/render/scene/args.hpp>
#include <metatron/device/command/context.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>
#include <metatron/device/opaque/sampler.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/network/remote/preview.hpp>
#include <metatron/core/math/bit.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/progress.hpp>
#include <random>

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

    auto Renderer::Impl::wave() noexcept -> void {
        auto& args = scene::Args::instance();
        auto addr = wired::Address{args.address};
        auto remote = !addr.host.empty();
        auto previewer = remote::Previewer{addr, "metatron"};

        command::Context::init();
        auto& scheduler = stl::scheduler::instance();
        auto render_queue = make_obj<command::Queue>(command::Type::render);
        auto transfer_queue = make_obj<command::Queue>(command::Type::transfer);

        auto upload_timelines = std::vector<obj<command::Timeline>>(scheduler.size());
        auto render_timeline = make_obj<command::Timeline>();
        auto transfer_timeline = make_obj<command::Timeline>();
        auto release_timeline = make_obj<command::Timeline>();
        auto copy_timeline = make_obj<command::Timeline>();
        auto network_timeline = make_obj<command::Timeline>();
        auto render_count = u64{0};
        auto scheduled_count = u64{0};

        auto resources = upload(render_queue.get(), upload_timelines);
        auto accel = build(render_queue.get(), upload_timelines, render_timeline.get(), render_count);
        auto render = render_queue->allocate();

        auto sampler = make_desc<opaque::Sampler>({opaque::Sampler::Mode::repeat});
        auto accessor = make_desc<opaque::Sampler>({opaque::Sampler::Mode::border});
        auto film = make_desc<opaque::Image>({&desc.film->image, opaque::Image::State::storable, command::Type::render});
        auto image = make_desc<opaque::Image>({&desc.film->image, opaque::Image::State::storable, command::Type::render});
        auto buffer = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::visible,
            .type = remote ? command::Type::transfer : command::Type::render,
            .size = math::prod(desc.film->image.size),
        });

        auto global_args = make_desc<shader::Argument>({"trace.global", command::Type::render});
        auto integrate_args = make_desc<shader::Argument>({"trace.integrate.in", command::Type::render});
        auto postprocess_args = make_desc<shader::Argument>({"trace.postprocess.in", command::Type::render});
        auto integrate = make_desc<shader::Pipeline>({"trace.integrate", {global_args.get(), integrate_args.get()}});
        auto postprocess = make_desc<shader::Pipeline>({"trace.postprocess", {global_args.get(), postprocess_args.get()}});

        auto global_args_encoder = encoder::Argument_Encoder{render.get(), global_args.get()};
        auto integrate_args_encoder = encoder::Argument_Encoder{render.get(), integrate_args.get()};
        auto postprocess_args_encoder = encoder::Argument_Encoder{render.get(), postprocess_args.get()};

        auto images_view = resources.images
        | std::views::transform([](auto&& x) -> opaque::Image::View { return *x; })
        | std::ranges::to<std::vector<opaque::Image::View>>();
        auto grids_view = resources.grids
        | std::views::transform([](auto&& x) -> opaque::Grid::View { return *x; })
        | std::ranges::to<std::vector<opaque::Grid::View>>();
        if (!images_view.empty()) global_args_encoder.bind("global.textures", {0, images_view});
        if (!grids_view.empty()) global_args_encoder.bind("global.grids", {0, grids_view});

        global_args_encoder.bind("global.accel", accel.get());
        global_args_encoder.bind("global.sampler", sampler.get());
        global_args_encoder.bind("global.accessor", accessor.get());
        global_args_encoder.upload();
        integrate_args_encoder.bind("in.image", *film);
        integrate_args_encoder.acquire("in.image", *film);
        integrate_args_encoder.upload();
        postprocess_args_encoder.bind("in.film", *film);
        postprocess_args_encoder.bind("in.image", *image);
        postprocess_args_encoder.acquire("in.image", *image);
        postprocess_args_encoder.upload();

        auto next = 1u;
        auto spp = desc.film->spp;
        auto range = uv2{0, 1};
        auto progress = stl::progress{spp};
        auto seed = std::random_device{}();
        stl::print("seed: 0x{:x}", seed);

        struct {
            Descriptor desc; u32 seed; uv2 range;
            math::Transform ct; buf<f32> fresnel;
        } in{
            std::move(desc), seed,
            {}, *entity<math::Transform>("/hierarchy/camera/render"),
            bsdf::Physical_Bsdf::fresnel_reflectance_table,
        };

        struct { uptr vectors; uptr volumes; } global{
            resources.vecarr->addr,
            resources.volarr ? resources.volarr->addr : 0
        };

        global_args_encoder.acquire("global", global);
        render->waits = {{render_timeline.get(), render_count}};
        render->signals = {{render_timeline.get(), ++render_count}};
        render_queue->submit(std::move(render));

        if (remote) scheduler.async_dispatch([&]{
            auto range = uv2{0, 1};
            auto next = 1u;
            auto count = 1;
            while (range[0] < spp) {
                copy_timeline->wait(count);
                previewer.update(desc.film->image, {buffer->ptr, buffer->size});
                network_timeline->signal(count);
                ++count; progress + (range[1] - range[0]);
                range = {range[1], range[1] + next};
                next = math::min(next * 2, desc.film->step);
            }
        });

        while (range[0] < spp) {
            auto integrate_cmd = render_queue->allocate();
            auto integrate_args_encoder = encoder::Argument_Encoder{integrate_cmd.get(), integrate_args.get()};
            auto integrate_encoder = encoder::Pipeline_Encoder{integrate_cmd.get(), integrate.get()};
            auto layout = uv3{math::align(film->width, 8u) / 8, math::align(film->height, 8u) / 8, 1};

            in.range = {range[0], range[1]};
            integrate_encoder.bind();
            integrate_args_encoder.acquire("in", in);
            integrate_encoder.dispatch(layout);

            integrate_cmd->waits = {{render_timeline.get(), render_count}};
            integrate_cmd->signals = {{render_timeline.get(), ++render_count}};
            render_queue->submit(std::move(integrate_cmd));

            auto postprocess_cmd = render_queue->allocate();
            auto postprocess_encoder = encoder::Pipeline_Encoder{postprocess_cmd.get(), postprocess.get()};
            if (!remote) {
                postprocess_encoder.bind();
                postprocess_encoder.dispatch(layout);
                postprocess_cmd->waits = {{render_timeline.get(), render_count}};
                postprocess_cmd->signals = {{render_timeline.get(), ++render_count}};
                render_queue->submit(std::move(postprocess_cmd));
            } else {
                auto transfer_cmd = transfer_queue->allocate();
                auto postprocess_args_encoder = encoder::Argument_Encoder{postprocess_cmd.get(), postprocess_args.get()};
                auto postprocess_release_encoder = encoder::Transfer_Encoder{postprocess_cmd.get()};
                auto transfer_encoder = encoder::Transfer_Encoder{transfer_cmd.get()};

                postprocess_args_encoder.acquire("in.image", *image);
                postprocess_encoder.bind();
                postprocess_encoder.dispatch(layout);
                postprocess_release_encoder.release(transfer_cmd.get(), *image);
                postprocess_cmd->waits = {
                    {render_timeline.get(), render_count},
                    {copy_timeline.get(), scheduled_count},
                };
                postprocess_cmd->signals = {
                    {render_timeline.get(), ++render_count},
                    {release_timeline.get(), scheduled_count + 1},
                };

                transfer_encoder.acquire(*image);
                transfer_encoder.copy(*buffer, *image);
                transfer_encoder.release(postprocess_cmd.get(), *image);
                transfer_cmd->waits = {
                    {release_timeline.get(), scheduled_count + 1},
                    {network_timeline.get(), scheduled_count},
                };
                transfer_cmd->signals = {{copy_timeline.get(), scheduled_count + 1}};

                render_queue->submit(std::move(postprocess_cmd));
                transfer_queue->submit(std::move(transfer_cmd));
                ++scheduled_count;
            }

            range[0] = range[1];
            range[1] = math::min(spp, range[1] + next);
            next = math::min(next * 2, desc.film->step);
        }

        if (!remote) {
            auto transfer_cmd = render_queue->allocate();
            auto transfer_encoder = encoder::Transfer_Encoder{transfer_cmd.get()};
            transfer_encoder.copy(*buffer, *image);
            transfer_cmd->waits = {{render_timeline.get(), render_count}};
            transfer_cmd->signals = {{render_timeline.get(), ++render_count}};
            render_queue->submit(std::move(transfer_cmd));
        }
        if (remote) network_timeline->wait(scheduled_count);
        else render_timeline->wait(render_count);
        auto data = std::span<byte const>{buffer->ptr, buffer->size};
        desc.film->image.to_path(args.output, desc.film->color_space, data);
    }
}
