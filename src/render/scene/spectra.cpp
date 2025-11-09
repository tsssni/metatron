#include <metatron/render/scene/serde.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/spectra/visible.hpp>
#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/resource/spectra/blackbody.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto spectra_init() noexcept -> void {
        using namespace spectra;

        auto& vec = stl::vector<Spectrum>::instance();
        vec.emplace_type<Constant_Spectrum>();
        vec.emplace_type<Rgb_Spectrum>();
        vec.emplace_type<Blackbody_Spectrum>();
        vec.emplace_type<Visible_Spectrum>();
        vec.emplace_type<Discrete_Spectrum>();

        auto name = std::to_array<std::string>({
            "zero", "one",
        });
        auto data = std::to_array<f32>({
            0.f, 1.f,
        });
        vec.reserve<Constant_Spectrum>(name.size());
        for (auto i = 0uz; i < name.size(); i++) {
            Spectrum::spectra.emplace(
                name[i],
                attach<Spectrum>(
                    ("/spectrum/" + name[i]) / et,
                    Constant_Spectrum{data[i]}
                )
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
        vec.reserve<Discrete_Spectrum>(std::ranges::count_if(spectra,
            [] (auto const& p) { return p.extension() == ".dspd"; })
        );
        vec.reserve<Visible_Spectrum>(std::ranges::count_if(spectra,
            [] (auto const& p) { return p.extension() == ".vspd"; })
        );

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

            auto push = [&]<typename S>(S&& spec) {
                auto lock = vec.lock<S>();
                auto entity = ("/spectrum/" + name) / et;
                auto handle = attach<Spectrum>(entity, std::forward<S>(spec));
                Spectrum::spectra.emplace(name, handle);
            };
            if (ext == ".vspd") push(Visible_Spectrum{{path}});
            else if (ext == ".dspd") push(Discrete_Spectrum{{path}});
        });

        MTT_DESERIALIZE(Spectrum
        , Constant_Spectrum
        , Rgb_Spectrum
        , Blackbody_Spectrum
        , Visible_Spectrum
        , Discrete_Spectrum
        );
    }
}
