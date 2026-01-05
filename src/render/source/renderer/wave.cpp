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
        auto shared_timeline = make_obj<command::Timeline>(true);
        auto network_timeline = make_obj<command::Timeline>(true);
        auto render_count = u64{0};
        auto scheduled_count = u64{0};

        auto resources = upload(render_queue.get(), upload_timelines);
        auto accel = build(render_queue.get(), upload_timelines, render_timeline.get(), render_count);
        auto render = render_queue->allocate({{render_timeline.get(), render_count}});

        auto sampler = make_desc<opaque::Sampler>({opaque::Sampler::Mode::repeat});
        auto film = make_desc<opaque::Image>({&desc.film->image, opaque::Image::State::storable, command::Type::render});
        auto image = make_desc<opaque::Image>({&desc.film->image, opaque::Image::State::storable, command::Type::render});
        auto buffer = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::visible,
            .type = remote ? command::Type::transfer : command::Type::render,
            .size = math::prod(desc.film->image.size),
        });

        auto resources_args = make_desc<shader::Argument>({"resources"});
        auto textures_args = make_desc<shader::Argument>({"textures"});
        auto grids_args = make_desc<shader::Argument>({"grids"});
        auto trace_args = make_desc<shader::Argument>({"trace"});
        auto post_args = make_desc<shader::Argument>({"postprocess"});

        auto integrate = make_desc<shader::Pipeline>({"trace.wave", {trace_args.get()
        , resources_args.get(), textures_args.get(), grids_args.get()}});
        auto postprocess = make_desc<shader::Pipeline>({"postprocess.wave", {post_args.get()}});

        auto textures_args_encoder = encoder::Argument_Encoder{render.get(), textures_args.get()};
        auto images_view = resources.images
        | std::views::transform([](auto&& x) -> opaque::Image::View { return *x; })
        | std::ranges::to<std::vector<opaque::Image::View>>();
        if (!images_view.empty()) textures_args_encoder.bind("textures.bindless", {0, images_view});
        textures_args_encoder.bind("textures.sampler", sampler.get());
        textures_args_encoder.upload();
        textures_args_encoder.submit();

        auto grids_args_encoder = encoder::Argument_Encoder{render.get(), grids_args.get()};
        auto grids_view = resources.grids
        | std::views::transform([](auto&& x) -> opaque::Grid::View { return *x; })
        | std::ranges::to<std::vector<opaque::Grid::View>>();
        if (!grids_view.empty()) grids_args_encoder.bind("grids.bindless", {0, grids_view});
        grids_args_encoder.upload();
        grids_args_encoder.submit();

        auto integrate_args_encoder = encoder::Argument_Encoder{render.get(), trace_args.get()};
        integrate_args_encoder.bind("constants.image", *film);
        integrate_args_encoder.acquire("constants.image", *film);
        integrate_args_encoder.upload();
        integrate_args_encoder.submit();

        auto postprocess_args_encoder = encoder::Argument_Encoder{render.get(), post_args.get()};
        postprocess_args_encoder.bind("constants.film", *film);
        postprocess_args_encoder.bind("constants.image", *image);
        postprocess_args_encoder.acquire("constants.image", *image);
        postprocess_args_encoder.upload();
        postprocess_args_encoder.submit();

        auto next = 1u;
        auto spp = desc.film->spp;
        auto range = uv2{0, 1};
        auto progress = stl::progress{spp};
        auto seed = std::random_device{}();
        stl::print("seed: 0x{:x}", seed);

        struct {
            Descriptor desc; u32 seed; uv2 range;
            math::Transform ct; buf<f32> fresnel;
        } entry{
            std::move(desc), seed,
            {}, *entity<math::Transform>("/hierarchy/camera/render"),
            bsdf::Physical_Bsdf::fresnel_reflectance_table,
        };

        struct { uptr vectors; uptr volumes; } global{
            resources.vecarr->addr,
            resources.volarr ? resources.volarr->addr : 0
        };

        auto resources_args_encoder = encoder::Argument_Encoder{render.get(), resources_args.get()};
        resources_args_encoder.bind("resources.accel", accel.get());
        resources_args_encoder.acquire("resources", global);
        resources_args_encoder.upload();
        resources_args_encoder.submit();
        render_queue->submit(std::move(render), {{render_timeline.get(), ++render_count}});

        auto threads = uv3{film->width, film->height, 1};
        auto group = uv3{8, 8, 1};
        auto future = scheduler.async_dispatch([&]{
            auto range = uv2{0, 1};
            auto count = 1;
            auto next = 1u;

            while (range[0] < spp) {
                shared_timeline->wait(count);
                if (remote) {
                    previewer.update(desc.film->image, {buffer->ptr, buffer->size});
                    network_timeline->signal(count);
                }
                ++count; progress + (range[1] - range[0]);
                range = {range[1], range[1] + next};
                next = math::min(next * 2, desc.film->stride);
            }

            if (!remote) {
                auto cmd = render_queue->allocate({{render_timeline.get(), render_count}});
                auto transfer = encoder::Transfer_Encoder{cmd.get()};
                transfer.copy(*buffer, *image);
                transfer.submit();
                render_queue->submit(std::move(cmd), {{shared_timeline.get(), count}});
                shared_timeline->wait(count);
            }
        });

        while (range[0] < spp) {
            auto cmd = render_queue->allocate({{render_timeline.get(), render_count}});
            auto args_encoder = encoder::Argument_Encoder{cmd.get(), trace_args.get()};
            entry.range = {range[0], range[1]};
            args_encoder.acquire("constants", entry);
            args_encoder.submit();

            auto render_encoder = encoder::Pipeline_Encoder{cmd.get(), integrate.get()};
            render_encoder.bind();
            render_encoder.dispatch(threads, group);
            render_encoder.submit();
            render_queue->submit(std::move(cmd), {{render_timeline.get(), ++render_count}});

            if (!remote) {
                auto cmd = render_queue->allocate({{render_timeline.get(), render_count}});
                auto render_encoder = encoder::Pipeline_Encoder{cmd.get(), postprocess.get()};
                render_encoder.bind();
                render_encoder.dispatch(threads, group);
                render_encoder.submit();
                render_queue->submit(std::move(cmd), {
                    {render_timeline.get(), ++render_count},
                    {shared_timeline.get(), ++scheduled_count},
                });
            } else {
                auto render_cmd = render_queue->allocate({
                    {render_timeline.get(), render_count},
                    {shared_timeline.get(), scheduled_count},
                });
                auto transfer_cmd = transfer_queue->allocate({
                    {render_timeline.get(), render_count + 1},
                    {network_timeline.get(), scheduled_count},
                });

                auto args_encoder = encoder::Argument_Encoder{render_cmd.get(), post_args.get()};
                args_encoder.acquire("constants.image", *image);
                args_encoder.submit();

                auto render_encoder = encoder::Pipeline_Encoder{render_cmd.get(), postprocess.get()};
                render_encoder.bind();
                render_encoder.dispatch(threads, group);
                render_encoder.submit();

                auto release_encoder = encoder::Transfer_Encoder{render_cmd.get()};
                release_encoder.release(transfer_cmd.get(), *image);
                release_encoder.submit();

                auto transfer_encoder = encoder::Transfer_Encoder{transfer_cmd.get()};
                transfer_encoder.acquire(*image);
                transfer_encoder.copy(*buffer, *image);
                transfer_encoder.release(render_cmd.get(), *image);
                transfer_encoder.submit();

                render_queue->submit(std::move(render_cmd), {{render_timeline.get(), ++render_count}});
                transfer_queue->submit(std::move(transfer_cmd), {{shared_timeline.get(), ++scheduled_count}});
            }

            range[0] = range[1];
            range[1] = math::min(spp, range[1] + next);
            next = math::min(next * 2, desc.film->stride);
        }

        future.wait();
        auto data = std::span<byte const>{buffer->ptr, buffer->size};
        desc.film->image.to_path(args.output, desc.film->color_space, data);
    }
}
