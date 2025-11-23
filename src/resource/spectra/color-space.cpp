#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>
#include <cstring>

namespace mtt::spectra {
    std::unordered_map<std::string, tag<Color_Space>> Color_Space::color_spaces;

    Color_Space::Color_Space(
        std::string_view name,
        cref<fv2> r, cref<fv2> g, cref<fv2> b,
        tag<spectra::Spectrum> illuminant,
        tag<Transfer_Function> transfer_function
    ) noexcept:
    illuminant(illuminant),
    illuminant_Y_integral(spectra::Spectrum::spectra["CIE-Y"] | illuminant),
    transfer_function(transfer_function) {
        // project color primaries and white point to Y=1
        auto w = ~illuminant;
        w /= math::sum(w);
        w = xyY_to_XYZ({w[0], w[1], 1.f});

        auto rgb = math::transpose(fm33{
            xyY_to_XYZ({r, 1.f}),
            xyY_to_XYZ({g, 1.f}),
            xyY_to_XYZ({b, 1.f}),
        });
        auto inv_rgb = math::inverse(rgb);
        auto c = inv_rgb | w;
        to_XYZ = rgb | fm33{c[0], c[1], c[2]};
        from_XYZ = math::inverse(to_XYZ);

        auto path = "metatron/color-space/" + std::string{name} + ".coeff";
        MTT_OPT_OR_CALLBACK(coeff, stl::filesystem::instance().find(path), {
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

        scale = table_res;
        table = table_res * table_res * table_res * 3 * 3;

        if (false
        || !file.read(mut<char>(scale.host), scale.bytelen)
        || !file.read(mut<char>(table.host), table.bytelen)) {
            std::println("{} coefficient could not read table", name);
            std::abort();
        }
        file.close();
    }
}
