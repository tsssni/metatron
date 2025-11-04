#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>

namespace mtt::scene {
    auto material_init() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto& vvec = stl::vector<texture::Vector_Texture>::instance();
        vvec.emplace_type<texture::Constant_Vector_Texture>();
        vvec.emplace_type<texture::Image_Vector_Texture>();

        auto& svec = stl::vector<texture::Spectrum_Texture>::instance();
        svec.emplace_type<texture::Constant_Spectrum_Texture>();
        svec.emplace_type<texture::Image_Spectrum_Texture>();
        svec.emplace_type<texture::Checkerboard_Texture>();

        auto& mvec = stl::vector<material::Material>::instance();
        mvec.emplace_type<material::Physical_Material>();
        mvec.emplace_type<material::Interface_Material>();

        for (auto& [spec, _]: spectra::Spectrum::spectra)
            hierarchy.attach<texture::Spectrum_Texture>(
                ("/texture/" + spec) / et,
                texture::Constant_Spectrum_Texture{
                    hierarchy.fetch<spectra::Spectrum>(("/spectrum/" + spec) / et),
                }
            );

        bsdf::Physical_Bsdf::init();
    }
}
