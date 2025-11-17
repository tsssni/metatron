#include <metatron/render/renderer/renderer.hpp>
#include <metatron/render/scene/args.hpp>
#include <metatron/network/remote/preview.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/progress.hpp>
#include <metatron/core/stl/memory.hpp>
#include <metatron/core/stl/print.hpp>
#include <OpenImageIO/imageio.h>

namespace mtt::renderer {
    struct Renderer::Impl final {
        Descriptor desc;
        Impl(rref<Descriptor> desc): desc(std::move(desc)) {}

        auto render() noexcept -> void {
            auto rd = std::random_device{};
            auto seed = rd();
            std::println("seed: {}", seed);
            std::println("buffer: {}", stl::memory{stl::stack::instance().size()});
            std::println("image: {}", [] {
                auto& vec = stl::vector<image::Image>::instance();
                auto size = 0uz;
                for (auto i = 0; i < vec.size(); i++)
                    for (auto j = 0; j < vec[i]->pixels.size(); j++)
                        size += vec[i]->pixels[j].size();
                return stl::memory{size};
            }());

            auto& args = scene::Args::instance();
            auto addr = wired::Address{args.address};

            auto ct = *scene::fetch<math::Transform>("/hierarchy/camera/render"_et);
            auto spp = desc.film.spp;
            auto depth = desc.film.depth;
            auto size = uzv2{desc.film.image->size};

            auto range = uzv2{0uz, addr.host.empty() ? spp : 1uz};
            auto progress = stl::progress{math::prod(size) * spp};
            auto trace = [&](cref<uzv2> px) {
                auto sp = *desc.sampler;
                for (auto n = range[0]; n < range[1]; ++n) {
                    sp->start({px, size, n, spp, 0uz, seed});
                    auto fixel = desc.film(desc.filter.data(), px, sp->generate_pixel_2d());
                    auto spec = spectra::Stochastic_Spectrum{sp->generate_1d()};
                    MTT_OPT_OR_CALLBACK(s, photo::Camera{}.sample(
                        desc.lens.data(), fixel.position, fixel.dxdy, sp->generate_2d()
                    ), {
                        std::println("ray generation failed");
                        std::abort();
                    });
                    s.ray_differential = ct ^ s.ray_differential;
                    s.default_differential = ct ^ s.default_differential;

                    auto ctx = monte_carlo::Context{
                        desc.accel.data(), desc.emitter.data(), sp, spec.lambda,
                        s.ray_differential, s.default_differential,
                        ct, px, n, depth,
                    };
                    MTT_OPT_OR_CALLBACK(Li, desc.integrator->sample(ctx), {
                        std::println("invalid value appears in pixel {} sample {}", px, n);
                        std::abort();
                    });
                    Li.value /= s.pdf;
                    fixel = Li;
                    ++progress;
                }
            };

            auto store = [
                path = args.output,
                fcs = desc.film.color_space
            ](cref<image::Image> img) {
                auto type = img.stride == 1
                ? OIIO::TypeDesc::UINT8 : OIIO::TypeDesc::FLOAT;
                auto spec = OIIO::ImageSpec{
                    i32(img.width), i32(img.height), i32(img.channels), type
                };

                auto cs_name = std::string{"sRGB"};
                for (auto const& [name, cs]: color::Color_Space::color_spaces)
                    if (fcs == cs) {cs_name = name; break;}
                spec.attribute("oiio::ColorSpace", cs_name);
                spec.attribute("planarconfig", "contig");

                auto out = OIIO::ImageOutput::create(std::string{path});
                if (!out || !out->open(std::string{path}, spec)) {
                    std::println("failed to create image file {}", path);
                    std::abort();
                }

                auto success = out->write_image(type, img.pixels.front().data());
                if (!success) {
                    std::println("failed to write image {}", path);
                    std::abort();
                }

                out->close();
            };

            auto next = 1uz;
            auto previewer = remote::Previewer{addr, "metatron"};
            auto& scheduler = stl::scheduler::instance();
            auto futures = std::vector<std::shared_future<void>>{};
            futures.reserve(spp / 64 + 8);
            futures.emplace_back(scheduler.async_dispatch([]{}));

            while (range[0] < spp) {
                stl::scheduler::instance().sync_parallel(size, trace);
                range[0] = range[1];
                range[1] = math::min(spp, range[1] + next);
                next = math::min(next * 2uz, 64uz);

                auto finished = range[0] == spp;
                auto image = *desc.film.image;
                scheduler.sync_parallel(
                    uzv2{image.size},
                    [&image](auto const& px) {
                        auto [i, j] = px;
                        auto pixel = fv4{image[i, j]};
                        pixel /= pixel[3];
                        image[i, j] = pixel;
                    }
                );
                if (finished) store(image);

                if (!addr.host.empty()) futures.push_back(scheduler.async_dispatch(
                    [
                        image = std::move(image),
                        &f = futures.back(),
                        &previewer
                    ] mutable {
                        f.wait();
                        previewer.update(std::move(image));
                    }
                ));
            }

            futures.back().wait();
            ~progress;
        }
    };

    Renderer::Renderer(rref<Descriptor> desc) noexcept:
    stl::capsule<Renderer>(std::move(desc)) {}

    auto Renderer::render() noexcept -> void {return impl->render();}
}
