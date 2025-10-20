#include <metatron/scene/daemon/spectrum.hpp>
#include <metatron/scene/compo/spectrum.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/resource/spectra/visible.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>
#include <sstream>

namespace mtt::daemon {
    auto Spectrum_Daemon::init() noexcept -> void {
        MTT_SERDE(Spectrum);
        auto& hierarchy = *ecs::Hierarchy::instance;
        auto& registry = hierarchy.registry;

        registry.emplace<poly<spectra::Spectrum>>(
            "/spectrum/zero"_et,
            make_poly<spectra::Spectrum, spectra::Constant_Spectrum>(0.f)
        );
        registry.emplace<poly<spectra::Spectrum>>(
            "/spectrum/one"_et,
            make_poly<spectra::Spectrum, spectra::Constant_Spectrum>(1.f)
        );

        for (auto const& name: std::to_array<std::string>({"zero", "one"})) {
            spectra::Spectrum::spectra.emplace(
                name,
                hierarchy.fetch<poly<spectra::Spectrum>>(("/spectrum/" + name) / et)
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
        stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{spectra.size()}, [&](auto idx) {
            auto path = spectra[idx[0]];
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

            auto read = [&](std::variant<std::array<f32, spectra::visible_range>, std::vector<math::Vector<f32, 2>>>&& data) -> void {
                auto idx = 0;
                auto file = std::ifstream{path};
                if (!file.is_open()) {
                    std::println("failed to open {}", path.string());
                    std::abort();
                }
                auto line = std::string{};

                while (std::getline(file, line)) {
                    if (line.empty() || line.front() == '#') continue;
                    
                    auto iss = std::istringstream{line};
                    auto wavelength = 0.f;
                    auto value = 0.f;

                    std::visit([&](auto&& data) {
                        using T = std::decay_t<decltype(data)>;
                        if constexpr (std::is_same_v<T, std::array<f32, spectra::visible_range>>) {
                            if (iss >> value) {
                                data[idx] = value;
                                ++idx;
                            }
                        } else {
                            if (iss >> wavelength >> value)
                                data.push_back(math::Vector<f32, 2>{wavelength, value});
                        }
                    }, data);
                }
                file.close();

                {
                    auto lock = std::lock_guard(mutex);
                    auto entity = ("/spectrum/" + name) / et;

                    std::visit([&](auto&& data) {
                        using T = std::decay_t<decltype(data)>;
                        if constexpr (std::is_same_v<T, std::array<f32, spectra::visible_range>>) {
                            registry.emplace<poly<spectra::Spectrum>>(
                                entity,
                                make_poly<spectra::Spectrum, spectra::Visible_Spectrum>(std::move(data))
                            );
                        } else {
                            registry.emplace<poly<spectra::Spectrum>>(
                                entity,
                                make_poly<spectra::Spectrum, spectra::Discrete_Spectrum>(std::move(data))
                            );
                        }
                    }, data);

                    spectra::Spectrum::spectra.emplace(
                        name,
                        hierarchy.fetch<poly<spectra::Spectrum>>(entity)
                    );
                }

            };

            if (ext == ".vspd") read(std::array<f32, spectra::visible_range>{});
            else read(std::vector<math::Vector<f32, 2>>{});
        });
    }

    auto Spectrum_Daemon::update() noexcept -> void {
        auto& hierarchy = *ecs::Hierarchy::instance;
        auto& registry = hierarchy.registry;

        auto view = registry.view<ecs::Dirty_Mark<compo::Spectrum>>()
        | std::views::filter([&](auto entity) {
            registry.remove<poly<spectra::Spectrum>>(entity);
            return registry.any_of<compo::Spectrum>(entity);
        })
        | std::ranges::to<std::vector<ecs::Entity>>();

        auto mutex = std::mutex{};
        stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{view.size()}, [&](auto idx) {
            auto entity = view[idx[0]];
            auto& spectrum = registry.get<compo::Spectrum>(entity);

            auto s = std::visit([&](auto&& compo) -> poly<spectra::Spectrum> {
                using T = std::decay_t<decltype(compo)>;
                if constexpr (std::is_same_v<T, compo::Constant_Spectrum>) {
                    return make_poly<spectra::Spectrum, spectra::Constant_Spectrum>(compo.x);
                } else if constexpr (std::is_same_v<T, compo::Rgb_Spectrum>) {
                    auto* color_space = &registry.get<color::Color_Space>(compo.color_space);
                    return color_space->to_spectrum(compo.c, compo.type);
                }
            },spectrum);

            {
                auto lock = std::lock_guard(mutex);
                registry.emplace<poly<spectra::Spectrum>>(entity, std::move(s));
            }
        });
        registry.clear<ecs::Dirty_Mark<compo::Spectrum>>();
    }
}
