#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::spectra {
    auto Spectrum::init() noexcept -> void {
        MTT_DESERIALIZE(
            Constant_Spectrum,
            Rgb_Spectrum,
            Blackbody_Spectrum,
            Visible_Spectrum,
            Discrete_Spectrum
        );

        auto spec_name = std::to_array<std::string>({
            "zero", "one",
        });
        auto data = std::to_array<f32>({
            0.f, 1.f,
        });
        for (auto i = 0uz; i < spec_name.size(); ++i)
            Spectrum::push<Constant_Spectrum>("/spectrum/" + spec_name[i], {data[i]});

        auto path = "spectra";
        auto spectra_dir = stl::filesystem::find(path);

        auto format = [](std::filesystem::path path) {
            auto name = std::string{};
            auto ext = path.extension();
            auto stem = std::filesystem::path(path).stem();
            auto extra_ext = std::to_array<std::string>({"eta", "k"});
            for (auto const& ext: extra_ext)
                if (stem.extension() == "." + ext) {
                    name = ext + "/";
                    stem = stem.stem();
                }
            name += stem;
            return name;
        };

        auto spectra = std::filesystem::directory_iterator(spectra_dir)
        | std::views::filter([](auto& it) {
            auto ext = it.path().extension();
            return it.is_regular_file() || ext == ".dspd" || ext == ".vspd";
        })
        | std::ranges::to<std::vector<std::filesystem::path>>();

        stl::scheduler::sync_parallel(uzv1{spectra.size()}, [&](auto idx) {
            auto [i] = idx;
            auto path = spectra[i];
            auto ext = path.extension();
            auto name = format(path);
            auto push = [&]<typename S>(S&& spec) {
                Spectrum::push<std::decay_t<S>>("/spectrum/" + name, std::forward<S>(spec));
            };
            if (ext == ".vspd") push(Visible_Spectrum{{path}});
            else if (ext == ".dspd") push(Discrete_Spectrum{{path}});
        });
    }
}
