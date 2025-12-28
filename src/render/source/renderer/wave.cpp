#include "renderer.hpp"
#include "resource.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>
#include <metatron/device/opaque/sampler.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/core/math/bit.hpp>
#include <metatron/core/stl/thread.hpp>
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
        command::Context::init();
        auto& scheduler = stl::scheduler::instance();
        auto render_queue = make_obj<command::Queue>(command::Type::render);
        auto transfer_queue = make_obj<command::Queue>(command::Type::transfer);

        auto upload_timelines = std::vector<obj<command::Timeline>>(scheduler.size());
        auto render_timeline = make_obj<command::Timeline>();
        auto transfer_timeline = make_obj<command::Timeline>();
        auto network_timline = make_obj<command::Timeline>();
        auto render_count = u64{0};
        auto transfer_count = u64{0};
        auto network_count = u64{0};

        auto resources = upload(render_queue.get(), upload_timelines);
        auto accel = build(
            render_queue.get(),
            upload_timelines,
            render_timeline.get(),
            render_count
        );

        auto transfer = transfer_queue->allocate();
        auto render = render_queue->allocate();

        auto sampler = make_obj<opaque::Sampler>(
        opaque::Sampler::Descriptor{
            .mode = opaque::Sampler::Mode::repeat,
        });
        auto accessor = make_obj<opaque::Sampler>(
        opaque::Sampler::Descriptor{
            .mode = opaque::Sampler::Mode::border,
            .border = opaque::Sampler::Border::transparent,
        });
        auto image = make_obj<opaque::Image>(
        opaque::Image::Descriptor{
            .image = &desc.film->image,
            .state = opaque::Image::State::storable,
            .type = command::Type::render,
        });
        auto buffer = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::visible,
            .type = command::Type::render,
            .size = math::prod(desc.film->image.size),
        });

        auto global_args = make_obj<shader::Argument>(
        shader::Argument::Descriptor{"trace.global", command::Type::render});
        auto integrate_args = make_obj<shader::Argument>(
        shader::Argument::Descriptor{"trace.integrate.in", command::Type::render});
        auto integrate = make_obj<shader::Pipeline>(
        shader::Pipeline::Descriptor{"trace.integrate", {global_args.get(), integrate_args.get()}});

        auto global_args_encoder = encoder::Argument_Encoder{render.get(), global_args.get()};
        auto integrate_args_encoder = encoder::Argument_Encoder{render.get(), integrate_args.get()};
        auto pipeline_encoder = encoder::Pipeline_Encoder{render.get(), integrate.get()};

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
        integrate_args_encoder.bind("in.image", *image);
        integrate_args_encoder.upload();
        pipeline_encoder.bind();

        auto spp = desc.film->spp;
        struct Integrate final {
            Descriptor desc;
            u32 seed;
            uv2 range;
            math::Transform ct;
            buf<f32> fresnel;
        } in{
            std::move(desc),
            std::random_device{}(),
            {0, spp},
            *entity<math::Transform>("/hierarchy/camera/render"),
            bsdf::Physical_Bsdf::fresnel_reflectance_table,
        };
        stl::print("seed: {}", in.seed);

        global_args_encoder.acquire("global", resources.resources->addr);
        integrate_args_encoder.acquire("in", in);
        integrate_args_encoder.acquire("in.image", *image);
        pipeline_encoder.dispatch({
            math::align(image->width, 8u) / 8,
            math::align(image->height, 8u) / 8,
        1});

        auto transfer_encoder = encoder::Transfer_Encoder{render.get()};
        transfer_encoder.copy(*buffer, *image);
        render->waits = {{render_timeline.get(), render_count}};
        render->signals = {{render_timeline.get(), ++render_count}};
        render_queue->submit(std::move(render));
        render_timeline->wait(render_count);
        std::memcpy(desc.film->image.pixels.front().data(), buffer->ptr, buffer->size);
        desc.film->image.to_path("build/test.exr", entity<spectra::Color_Space>("/color-space/sRGB"));
    }
}
