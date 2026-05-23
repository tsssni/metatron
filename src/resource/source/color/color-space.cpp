#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/serde/serde.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>
#include <cstring>

namespace mtt::color {
    auto proxy::Color_Space::init() noexcept -> void {
        Transfer_Function::init();

        auto& cvec = stl::vector<color::Color_Space>::instance();
        auto cs_name = std::to_array<std::string>({"sRGB"});
        auto red_primitive = std::to_array<fv2>({{0.64f, 0.33f}});
        auto green_primitive = std::to_array<fv2>({{0.30f, 0.60f}});
        auto blue_primitive = std::to_array<fv2>({{0.15f, 0.06f}});
        auto white_point = std::to_array<std::string>({"CIE-D65"});
        auto transfer_function_name = std::to_array<std::string>({"Rec709"});

        stl::scheduler::instance().sync_parallel(uzv1{cs_name.size()}, [&](auto idx) {
            auto i = idx[0];
            cvec.push("/color-space/" + cs_name[i], color::Color_Space{
                cs_name[i],
                red_primitive[i],
                green_primitive[i],
                blue_primitive[i],
                spectra::Spectrum::entity("/spectrum/" + white_point[i]),
                Transfer_Function::entity("/transfer-function/" + transfer_function_name[i]),
            });
        });
    }

    Color_Space::Color_Space(
        std::string_view name,
        cref<fv2> r, cref<fv2> g, cref<fv2> b, u32 i,
        Transfer_Function transfer_function
    ) noexcept:
    illuminant(i),
    illuminant_Y_integral(spectra::Spectrum::entity("/spectrum/CIE-Y") | spectra::Spectrum{i}),
    transfer_function(transfer_function) {
        // project color primaries and white point to Y=1
        auto illuminant = spectra::Spectrum{this->illuminant};
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

        auto path = "color-space/" + std::string{name} + ".coeff";
        auto coeff = stl::filesystem::find(path);
        auto file = std::ifstream{coeff, std::ios::binary};

        auto header = std::string(4, '\0');
        if (false
        || !file.read(header.data(), 4)
        || header != "SPEC")
            stl::abort("{} coefficient has wrong header", name);

        if (!file.read(mut<char>(&table_res), sizeof(i32)))
            stl::abort("{} coefficient could not read table resolution", name);
        scale = table_res;
        table = table_res * table_res * table_res * 3 * 3;

        if (false
        || !file.read(mut<char>(scale.ptr), scale.bytelen)
        || !file.read(mut<char>(table.ptr), table.bytelen))
            stl::abort("{} coefficient could not read table", name);
        file.close();
    }
}
