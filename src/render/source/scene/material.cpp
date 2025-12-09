#include <metatron/render/scene/serde.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>

namespace glz {
    template<>
    struct meta<mtt::texture::Image_Distribution> {
        using enum mtt::texture::Image_Distribution;
        auto constexpr static value = glz::enumerate(none, uniform, spherical);
    };
}

namespace mtt::scene {
    auto material_init() noexcept -> void {
        using namespace texture;
        using namespace material;
        using namespace accel;

        MTT_DESERIALIZE(Vector_Texture
        , Constant_Vector_Texture
        , Image_Vector_Texture);
        MTT_DESERIALIZE(Spectrum_Texture
        , Constant_Spectrum_Texture
        , Image_Spectrum_Texture
        , Checkerboard_Texture);
        MTT_DESERIALIZE(Material
        , Physical_Material
        , Interface_Material);
        MTT_DESERIALIZE(Divider);

        bsdf::Physical_Bsdf::init();
        for (auto& [spec, _]: spectra::Spectrum::spectra)
            attach<Spectrum_Texture>(
                ("/texture/" + spec) / et,
                Constant_Spectrum_Texture{
                    fetch<spectra::Spectrum>(("/spectrum/" + spec) / et),
                }
            );
    }
}
