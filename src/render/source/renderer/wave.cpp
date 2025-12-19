#include "renderer.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/encoder/accel.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/opaque/sampler.hpp>
#include <metatron/device/opaque/accel.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::renderer {
    struct Resources final {
        std::vector<obj<opaque::Buffer>> buffers;
        std::vector<obj<opaque::Buffer>> vectors{};
        obj<opaque::Buffer> resources;
        std::vector<obj<opaque::Image>> images;
        std::vector<obj<opaque::Grid>> grids;
    };

    auto upload(
        mut<command::Queue> queue,
        ref<std::vector<obj<command::Timeline>>> timelines
    ) noexcept -> Resources {
        auto& scheduler = stl::scheduler::instance();
        auto& stack = stl::stack::instance();
        auto& vector = stl::vector<void>::instance();

        auto buffers = std::vector<obj<opaque::Buffer>>{};
        auto vectors = std::vector<obj<opaque::Buffer>>{};
        auto resources = obj<opaque::Buffer>{};
        auto images = std::vector<obj<opaque::Image>>{};
        auto grids = std::vector<obj<opaque::Grid>>{};

        auto& textures = stl::vector<muldim::Image>::instance();
        auto& volumes = stl::vector<muldim::Grid>::instance();
        auto vsize = vector.size();
        auto bsize = stack.bufs.size();
        auto isize = textures.size();
        auto gsize = volumes.size();
        auto sequence = std::vector<std::vector<byte>>(vsize);
        auto addresses = std::vector<uptr>(vsize, 0);
        buffers = std::vector<obj<opaque::Buffer>>(bsize);
        vectors = std::vector<obj<opaque::Buffer>>(vsize);
        images = std::vector<obj<opaque::Image>>(isize);
        grids = std::vector<obj<opaque::Grid>>(gsize);

        scheduler.sync_parallel(uzv1{scheduler.size()}, [
            &,
            bc = std::make_shared<std::atomic<u32>>(0),
            vc = std::make_shared<std::atomic<u32>>(0),
            ic = std::make_shared<std::atomic<u32>>(0),
            gc = std::make_shared<std::atomic<u32>>(0)
        ](auto) {
            auto cmd = queue->allocate();
            auto transfer = encoder::Transfer_Encoder{cmd.get()};

            auto i = 0;
            while ((i = bc->fetch_add(1, std::memory_order::relaxed)) < bsize) {
                auto& buf = stack.bufs[i];
                if (buf->idx != i) continue;

                auto desc = opaque::Buffer::Descriptor{
                    .ptr = buf->ptr,
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = buf->bytelen,
                };
                auto buffer = make_obj<opaque::Buffer>(desc);
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                stack.release(buf);

                buf->ptr = mut<byte>(buffer->addr);
                buf->handle = uptr(buffer.get());
                buf->idx = math::maxv<u32>;
                buffers[i] = std::move(buffer);
            }

            i = 0;
            while ((i = vc->fetch_add(1, std::memory_order::acq_rel)) < vsize) {
                auto& vec = vector.storage[i];
                sequence[i] = vec.pack();
                if (sequence[i].empty()) continue;

                auto desc = opaque::Buffer::Descriptor{
                    .ptr = sequence[i].data(),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = sequence[i].size(),
                };
                auto buffer = make_obj<opaque::Buffer>(desc);
                transfer.upload(*buffer);
                transfer.persist(*buffer);
                addresses[i] = buffer->addr;
                vectors[i] = std::move(buffer);
            }

            if (scheduler.index() == 0) {
                auto desc = opaque::Buffer::Descriptor{
                    .ptr = mut<byte>(addresses.data()),
                    .state = opaque::Buffer::State::local,
                    .type = command::Type::render,
                    .size = addresses.size() * sizeof(uptr),
                };
                resources = make_obj<opaque::Buffer>(desc);
                encoder::Transfer_Encoder{cmd.get()}.upload(*resources);
            }

            i = 0;
            while ((i = ic->fetch_add(1, std::memory_order::relaxed)) < isize) {
                images[i] = make_obj<opaque::Image>(
                opaque::Image::Descriptor{
                    .image = textures[i],
                    .state = opaque::Image::State::samplable,
                    .type = command::Type::render,
                });
                transfer.upload(*images[i]);
                transfer.persist(*images[i]);
            }

            i = 0;
            while ((i = gc->fetch_add(1, std::memory_order::relaxed)) < gsize) {
                grids[i] = make_obj<opaque::Grid>(
                opaque::Grid::Descriptor{
                    .grid = volumes[i],
                    .state = opaque::Grid::State::readonly,
                    .type = command::Type::render,
                });
                transfer.upload(*grids[i]);
                transfer.persist(*grids[i]);
            }

            timelines[scheduler.index()] = make_obj<command::Timeline>();
            cmd->signals = {{timelines[scheduler.index()].get(), 1}};
            queue->submit(std::move(cmd));
        });

        return Resources{
            std::move(buffers),
            std::move(vectors),
            std::move(resources),
            std::move(images),
            std::move(grids),
        };
    }

    template<typename T>
    auto collect(
        ref<std::vector<opaque::Acceleration::Primitive>> primitives,
        ref<std::vector<u32>> counts
    ) noexcept -> void {
        auto& shapes = stl::vector<shape::Shape>::instance();
        for (auto i = 0; i < shapes.size<T>(); ++i)
            if constexpr (std::is_same_v<T, shape::Mesh>)
                primitives.push_back({
                    .type = opaque::Acceleration::Primitive::Type::mesh,
                    .mesh = shapes.get<T>(i),
                });
            else {
                auto prim = opaque::Acceleration::Primitive{
                    .type = opaque::Acceleration::Primitive::Type::aabb,
                };
                for (auto j = 0; j < shapes.get<T>(i)->size(); ++j)
                    prim.aabbs.push_back(shapes.get<T>(i)->bounding_box(fm4{1.f}, j));
                primitives.push_back(prim);
            }
        counts.push_back(counts.back() + shapes.size<T>());
    }

    auto build(
        mut<command::Queue> queue,
        cref<std::vector<obj<command::Timeline>>> uploads,
        mut<command::Timeline> timeline,
        ref<u64> count
    ) noexcept -> obj<opaque::Acceleration> {
        auto& scheduler = stl::scheduler::instance();
        auto& dividers = stl::vector<accel::Divider>::instance();
        auto& shapes = stl::vector<shape::Shape>::instance();
        auto counts = std::vector<u32>{0};
        auto primitives = std::vector<opaque::Acceleration::Primitive>{};
        auto instances = std::vector<opaque::Acceleration::Instance>{};

        collect<shape::Mesh>(primitives, counts);
        collect<shape::Sphere>(primitives, counts);

        for (auto i = 0; i < dividers.size(); ++i) {
            auto div = dividers[i];
            auto type = div->shape.type();
            auto idx = div->shape.index();
            auto tlas = opaque::Acceleration::Instance{
                .idx = counts[type] + idx,
                .transform = div->local_to_render->transform,
            };
            instances.push_back(tlas);
        }
        auto accel = make_obj<opaque::Acceleration>(
        opaque::Acceleration::Descriptor{
            .primitives = std::move(primitives),
            .instances = std::move(instances),
        });

        auto builder = queue->allocate();
        auto encoder = encoder::Acceleration_Encoder{builder.get(), accel.get()};
        encoder.build();
        encoder.persist();
        for (auto i = 0; i < scheduler.size(); ++i)
            builder->waits.push_back({uploads[i].get(), 1});
        builder->signals = {{timeline, ++count}};
        queue->submit(std::move(builder));
        return accel;
    }

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
        render_timeline->wait(render_count);

        auto transfer = transfer_queue->allocate();
        auto render = render_queue->allocate();

        auto sampler = make_obj<opaque::Sampler>(
        opaque::Sampler::Descriptor{
            .mode = opaque::Sampler::Mode::repeat,
        });
        auto image = make_obj<opaque::Image>(
        opaque::Image::Descriptor{
            .image = &desc.film.image,
            .state = opaque::Image::State::storable,
            .type = command::Type::render,
        });
        auto buffer = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::visible,
            .type = command::Type::render,
            .size = math::prod(desc.film.image.size),
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
        auto images_view = std::vector<opaque::Image::View>(resources.images.size());
        for (auto i = 0; i < resources.images.size(); ++i)
            images_view[i] = *resources.images[i];
        global_args_encoder.bind("global.accel", accel.get());
        global_args_encoder.bind("global.sampler", sampler.get());
        global_args_encoder.bind("global.textures", {0, images_view});
        global_args_encoder.upload();
        integrate_args_encoder.bind("in.image", *image);
        integrate_args_encoder.upload();
        pipeline_encoder.bind();

        struct Integrate final {
            math::Transform ct;
            photo::Film film;
        } in{
            *entity<math::Transform>("/hierarchy/camera/render"),
            std::move(desc.film),
        };
        global_args_encoder.acquire("global", resources.resources->addr);
        integrate_args_encoder.acquire("in", in);
        integrate_args_encoder.acquire("in.image", *image);
        pipeline_encoder.dispatch({
            math::align(image->width, 8) / 8,
            math::align(image->height, 8) / 8,
        1});

        auto transfer_encoder = encoder::Transfer_Encoder{render.get()};
        transfer_encoder.copy(*buffer, *image);
        render->waits = {{render_timeline.get(), render_count}};
        render->signals = {{render_timeline.get(), ++render_count}};
        render_queue->submit(std::move(render));
        render_timeline->wait(render_count);
        std::memcpy(desc.film.image.pixels.front().data(), buffer->ptr, buffer->size);
        desc.film.image.to_path("build/test.exr", entity<spectra::Color_Space>("/color-space/sRGB"));
    }
}
