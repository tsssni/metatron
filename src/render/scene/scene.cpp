#include <metatron/render/scene/scene.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/spectra/visible.hpp>
#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/resource/spectra/blackbody.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto init() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        [&] {
            auto& vec = stl::poly_vector<spectra::Spectrum>::instance();
            vec.emplace_type<spectra::Constant_Spectrum>();
            vec.emplace_type<spectra::Rgb_Spectrum>();
            vec.emplace_type<spectra::Visible_Spectrum>();
            vec.emplace_type<spectra::Discrete_Spectrum>();
            vec.emplace_type<spectra::Blackbody_Spectrum>();

            auto e = "/spectrum/zero"_et;
            hierarchy.attach<spectra::Spectrum>(
                "/spectrum/zero"_et,
                spectra::Constant_Spectrum{0.f}
            );
            hierarchy.attach<spectra::Spectrum>(
                "/spectrum/one"_et,
                spectra::Constant_Spectrum{1.f}
            );
            for (auto const& name: std::to_array<std::string>({"zero", "one"})) {
                spectra::Spectrum::spectra.emplace(
                    name,
                    hierarchy.fetch<spectra::Spectrum>(("/spectrum/" + name) / et).data()
                );
            }

            MTT_OPT_OR_CALLBACK(spectra_dir, stl::filesystem::instance().find("spectra"), {
                std::println("spectra not exist");
                std::abort();
            });

            auto spectra = std::filesystem::directory_iterator(spectra_dir)
            | std::views::filter([](auto& it) { return true
                && it.is_regular_file()
                && (it.path().extension() == ".dspd" || it.path().extension() == ".vspd");
            })
            | std::ranges::to<std::vector<std::filesystem::path>>();

            auto mutex = std::mutex{};
            auto grid = math::Vector<usize, 1>{spectra.size()};
            stl::scheduler::instance().sync_parallel(grid, [&](auto idx) {
                auto [i] = idx;
                auto path = spectra[i];
                auto ext = path.extension();

                auto name = std::string{};
                auto extra_ext = std::to_array<std::string>({"eta", "k"});
                auto stem = std::filesystem::path(path).stem();
                for (auto const& ext: extra_ext)
                    if (stem.extension() == "." + ext) {
                        name = ext + "/";
                        stem = stem.stem();
                    }
                name += stem;

                auto attach = [&]<typename S>(S&& spec) {
                    auto lock = std::lock_guard(mutex);
                    auto entity = ("/spectrum/" + name) / et;
                    auto handle = hierarchy.attach<spectra::Spectrum>(entity, std::forward<S>(spec));
                    spectra::Spectrum::spectra.emplace(name, handle.data());
                };
                if (ext == ".vspd") attach(spectra::Visible_Spectrum{{path}});
                else if (ext == ".dspd") attach(spectra::Discrete_Spectrum{{path}});
            });
        }();

        [&] {
            auto& vec = stl::poly_vector<color::Color_Space>::instance();
            vec.emplace_type<color::Color_Space>();

            auto name = std::to_array<std::string>({
                "sRGB",
            });
            auto red_primitive = std::to_array<math::Vector<f32, 2>>({
                {0.64f, 0.33f},
            });
            auto green_primitive = std::to_array<math::Vector<f32, 2>>({
                {0.30f, 0.60f},
            });
            auto blue_primitive = std::to_array<math::Vector<f32, 2>>({
                {0.15f, 0.06f},
            });
            auto white_point = std::to_array<std::string>({
                "CIE-D65",
            });
            auto transfer = std::to_array({
                [](f32 x) -> f32 {
                    if (x <= 0.0031308f) return 12.92f * x;
                    else return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
                }
            });
            auto linearize = std::to_array({
                [](f32 x) -> f32 {
                    if (x <= 0.04045f) return x / 12.92f;
                    else return std::pow((x + 0.055f) / 1.055f, 2.4f);
                }
            });

            auto mutex = std::mutex{};
            auto grid = math::Vector<usize, 1>{name.size()};
            stl::scheduler::instance().sync_parallel(grid, [&](auto idx) {
                auto i = idx[0];
                auto entity = ("/color-space/" + name[i]) / et;
                auto white = ("/spectrum/" + white_point[i]) / et;

                auto cs = color::Color_Space{
                    name[i],
                    red_primitive[i],
                    green_primitive[i],
                    blue_primitive[i],
                    hierarchy.fetch<spectra::Spectrum>(white),
                    transfer[i],
                    linearize[i]
                };

                {
                    auto lock = std::lock_guard(mutex);
                    hierarchy.attach<color::Color_Space>(entity, std::move(cs));
                    auto cs = hierarchy.fetch<color::Color_Space>(entity);
                    color::Color_Space::color_spaces[name[i]] = cs.data();
                }
            });
        }();
    }

    auto test() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();
        auto handle = hierarchy.fetch<spectra::Spectrum>("/spectrum/k/K" / et);
        std::println("{} {}", u32(handle), (*handle.data())(305));
    }
}
