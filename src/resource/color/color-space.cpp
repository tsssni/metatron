#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>
#include <cstring>

namespace mtt::color {
    std::unordered_map<std::string, proxy<Color_Space>> Color_Space::color_spaces;

    Color_Space::Color_Space(
        std::string_view name,
        math::Vector<f32, 2> const& r_chroma,
        math::Vector<f32, 2> const& g_chroma,
        math::Vector<f32, 2> const& b_chroma,
        proxy<spectra::Spectrum> illuminant,
        proxy<Transfer_Function> transfer_function
    ): 
    illuminant(illuminant),
    illuminant_Y_integral(spectra::Spectrum::spectra["CIE-Y"].data() | illuminant.data()),
    transfer_function(transfer_function) {
        // project color primaries and white point to Y=1
        auto w = ~illuminant.data();
        w /= math::sum(w);
        w = xyY_to_XYZ({w[0], w[1], 1.f});
        auto r = xyY_to_XYZ({r_chroma, 1.f});
        auto g = xyY_to_XYZ({g_chroma, 1.f});
        auto b = xyY_to_XYZ({b_chroma, 1.f});

        auto rgb = math::transpose(math::Matrix<f32, 3, 3>{r, g, b});
        auto inv_rgb = math::inverse(rgb);
        auto c = inv_rgb | w;
        to_XYZ = rgb | math::Matrix<f32, 3, 3>{c[0], c[1], c[2]};
        from_XYZ = math::inverse(to_XYZ);

        MTT_OPT_OR_CALLBACK(coeff, stl::filesystem::instance().find("color-space/" + std::string{name} + ".coeff"), {
            std::println("{} coefficient not exists", name);
            std::abort();
        });
        auto file = std::ifstream{coeff, std::ios::binary};

        auto header = std::string(4, '\0');
        if (false
        || !file.read(header.data(), 4)
        || header != "SPEC") {
            std::println("{} coefficient has wrong header", name);
            std::abort();
        }

        if (!file.read(mut<char>(&table_res), sizeof(i32))) {
            std::println("{} coefficient could not read table resolution", name);
            std::abort();
        }

        auto scale_size = table_res;
        auto data_size = table_res * table_res * table_res * 3 * 3;
        scale.resize(scale_size);
        table.resize(data_size);

        if (false
        || !file.read(mut<char>(scale.data()), scale_size * sizeof(f32))
        || !file.read(mut<char>(table.data()), data_size * sizeof(f32))) {
            std::println("{} coefficient could not read table", name);
            std::abort();
        }
        file.close();
    }
}
