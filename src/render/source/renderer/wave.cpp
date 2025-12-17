#include "renderer.hpp"
#include <metatron/device/command/context.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/device/encoder/transfer.hpp>
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
    auto Renderer::Impl::wave() noexcept -> void {
        command::Context::init();
        auto& scheduler = stl::scheduler::instance();
        auto& stack = stl::stack::instance();
        auto& vector = stl::vector<void>::instance();
        auto render_queue = make_obj<command::Queue>(command::Type::render);
        auto transfer_queue = make_obj<command::Queue>(command::Type::transfer);

        auto upload_timelines = std::vector<obj<command::Timeline>>(scheduler.size());
        auto render_timeline = make_obj<command::Timeline>();
        auto transfer_timeline = make_obj<command::Timeline>();
        auto network_timline = make_obj<command::Timeline>();
        auto render_count = u64{0};
        auto transfer_count = u64{0};
        auto network_count = u64{0};

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
            auto cmd = render_queue->allocate();
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
                stl::stack::instance().release(buf);

                buf->ptr = mut<byte>(buffer->addr);
                buf->handle = uptr(buffer.get());
                buf->idx = math::maxv<u32>;
                buffers[i] = std::move(buffer);
            }

            i = 0;
            while ((i = vc->fetch_add(1, std::memory_order::relaxed)) < vsize) {
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

            upload_timelines[scheduler.index()] = make_obj<command::Timeline>();
            cmd->signals = {{upload_timelines[scheduler.index()].get(), 1}};
            render_queue->submit(std::move(cmd));
        });

        auto accel = [&] {
            auto cmd = render_queue->allocate();
            auto& dividers = stl::vector<accel::Divider>::instance();
            auto& shapes = stl::vector<shape::Shape>::instance();
            auto primitives = std::vector<opaque::Acceleration::Primitive>{};
            auto instances = std::vector<opaque::Acceleration::Instance>{};
            for (auto i = 0; i < dividers.size(); ++i) {
                auto div = dividers[i];
                auto tlas = opaque::Acceleration::Instance{div->local_to_render->transform};
                auto blas = opaque::Acceleration::Primitive{};
                if (shapes.is<shape::Mesh>(div->shape))
                    blas.mesh = shapes.get<shape::Mesh>(div->shape.index());
                else blas.aabb = div->shape->bounding_box(fm4{1.f}, 0);
                instances.push_back(tlas);
                primitives.push_back(blas);
            }
            auto accel = make_obj<opaque::Acceleration>(
            opaque::Acceleration::Descriptor{
                .primitives = std::move(primitives),
                .instances = std::move(instances),
            });

            auto desc = opaque::Buffer::Descriptor{
                .ptr = mut<byte>(addresses.data()),
                .state = opaque::Buffer::State::local,
                .type = command::Type::render,
                .size = addresses.size() * sizeof(uptr),
            };
            resources = make_obj<opaque::Buffer>(desc);
            encoder::Transfer_Encoder{cmd.get()}.upload(*resources);

            for (auto i = 0; i < scheduler.size(); ++i)
                cmd->waits.push_back({upload_timelines[i].get(), 1});
            cmd->signals = {{render_timeline.get(), ++render_count}};
            render_queue->submit(std::move(cmd));
            return std::move(accel);
        }();

        auto transfer = transfer_queue->allocate();
        auto render = render_queue->allocate();

        auto film = muldim::Image{};
        film.width = images[0]->width;
        film.height = images[0]->height;
        film.channels = 4;
        film.stride = 4;

        auto size = math::prod(film.size);
        film.pixels.resize(1);
        film.pixels.front().resize(math::prod(film.size));

        auto sampler = make_obj<opaque::Sampler>(
        opaque::Sampler::Descriptor{
            .mode = opaque::Sampler::Mode::repeat,
        });
        auto image = make_obj<opaque::Image>(
        opaque::Image::Descriptor{
            .image = &film,
            .state = opaque::Image::State::storable,
            .type = command::Type::render,
        });
        auto buffer = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::visible,
            .type = command::Type::render,
            .size = size,
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
        auto images_view = std::vector<opaque::Image::View>(images.size());
        for (auto i = 0; i < images.size(); ++i)
            images_view[i] = *images[i];
        global_args_encoder.bind("global.sampler", sampler.get());
        global_args_encoder.bind("global.textures", {0, images_view});
        global_args_encoder.upload();
        integrate_args_encoder.bind("in.film", *image);
        integrate_args_encoder.upload();
        pipeline_encoder.bind();

        struct Integrate final {
            u32 idx;
            u32 offset;
        } in;
        in.idx = stl::vector<texture::Spectrum_Texture>::instance().index<texture::Image_Spectrum_Texture>();
        in.offset = 8;
        global_args_encoder.acquire("global", resources->addr);
        integrate_args_encoder.acquire("in", in);
        integrate_args_encoder.acquire("in.film", *image);
        pipeline_encoder.dispatch({
            math::align(film.width, 8) / 8,
            math::align(film.height, 8) / 8,
        1});

        auto transfer_encoder = encoder::Transfer_Encoder{render.get()};
        transfer_encoder.copy(*buffer, *image);
        render->waits = {{render_timeline.get(), render_count}};
        render->signals = {{render_timeline.get(), ++render_count}};
        render_queue->submit(std::move(render));
        render_timeline->wait(render_count);
        std::memcpy(film.pixels.front().data(), buffer->ptr, size);
        film.to_path("build/test.exr", entity<spectra::Color_Space>("/color-space/sRGB"));
    }
}
