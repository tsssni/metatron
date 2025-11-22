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

        auto& svec = stl::vector<Spectrum>::instance();
        MTT_DESERIALIZE(Spectrum
        , Constant_Spectrum
        , Rgb_Spectrum
        , Blackbody_Spectrum
        , Visible_Spectrum
        , Discrete_Spectrum
        );

        auto spec_name = std::to_array<std::string>({
            "zero", "one",
        });
        auto data = std::to_array<f32>({
            0.f, 1.f,
        });
        svec.reserve<Constant_Spectrum>(spec_name.size());
        for (auto i = 0uz; i < spec_name.size(); i++) {
            Spectrum::spectra.emplace(
                spec_name[i],
                attach<Spectrum>(
                    ("/spectrum/" + spec_name[i]) / et,
                    Constant_Spectrum{data[i]}
                )
            );
        }

        auto path = "metatron/spectra";
        MTT_OPT_OR_CALLBACK(spectra_dir, stl::filesystem::instance().find(path), {
            std::println("spectra not exist");
            std::abort();
        });

        auto spectra = std::filesystem::directory_iterator(spectra_dir)
        | std::views::filter([](auto& it) { return true
        && it.is_regular_file()
        && (it.path().extension() == ".dspd" || it.path().extension() == ".vspd");
        })
        | std::ranges::to<std::vector<std::filesystem::path>>();
        svec.reserve<Discrete_Spectrum>(std::ranges::count_if(spectra,
            [] (auto const& p) { return p.extension() == ".dspd"; })
        );
        svec.reserve<Visible_Spectrum>(std::ranges::count_if(spectra,
            [] (auto const& p) { return p.extension() == ".vspd"; })
        );

        stl::scheduler::instance().sync_parallel(uzv1{spectra.size()}, [&](auto idx) {
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
                auto lock = svec.lock();
                auto entity = ("/spectrum/" + name) / et;
                auto handle = attach<Spectrum>(entity, std::forward<S>(spec));
                Spectrum::spectra.emplace(name, handle);
            };
            if (ext == ".vspd") push(Visible_Spectrum{{path}});
            else if (ext == ".dspd") push(Discrete_Spectrum{{path}});
        });

        auto& tvec = stl::vector<Color_Space::Transfer_Function>::instance();
        auto& cvec = stl::vector<Color_Space>::instance();

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

        for (auto i = 0uz; i < transfer.size(); i++)
            tvec.emplace_back(transfer[i], linearize[i]);

        auto cs_name = std::to_array<std::string>({
            "sRGB",
        });
        auto red_primitive = std::to_array<fv2>({
            {0.64f, 0.33f},
        });
        auto green_primitive = std::to_array<fv2>({
            {0.30f, 0.60f},
        });
        auto blue_primitive = std::to_array<fv2>({
            {0.15f, 0.06f},
        });
        auto white_point = std::to_array<std::string>({
            "CIE-D65",
        });
        auto transfer_function = std::to_array<u32>({
            0,
        });

        stl::scheduler::instance().sync_parallel(uzv1{cs_name.size()}, [&](auto idx) {
            auto i = idx[0];
            auto entity = ("/color-space/" + spec_name[i]) / et;
            auto white = ("/spectrum/" + white_point[i]) / et;

            auto cs = Color_Space{
                cs_name[i],
                red_primitive[i],
                green_primitive[i],
                blue_primitive[i],
                fetch<spectra::Spectrum>(white),
                tag<Color_Space::Transfer_Function>{transfer_function[i]},
            };

            {
                auto lock = cvec.lock();
                attach<Color_Space>(entity, std::move(cs));
                auto cs = fetch<Color_Space>(entity);
                Color_Space::color_spaces[cs_name[i]] = cs;
            }
        });
    }
}
