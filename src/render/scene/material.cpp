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
        auto constexpr static value = glz::enumerate(
            none,
            uniform,
            spherical
        );
    };
}

namespace mtt::scene {
    auto material_init() noexcept -> void {
        using namespace texture;
        using namespace material;
        using namespace accel;
        bsdf::Physical_Bsdf::init();

        auto& vvec = stl::vector<Vector_Texture>::instance();
        vvec.emplace_type<Constant_Vector_Texture>();
        vvec.emplace_type<Image_Vector_Texture>();

        auto& svec = stl::vector<Spectrum_Texture>::instance();
        svec.emplace_type<Constant_Spectrum_Texture>();
        svec.emplace_type<Image_Spectrum_Texture>();
        svec.emplace_type<Checkerboard_Texture>();

        auto& mvec = stl::vector<Material>::instance();
        mvec.emplace_type<Physical_Material>();
        mvec.emplace_type<Interface_Material>();

        for (auto& [spec, _]: spectra::Spectrum::spectra)
            attach<Spectrum_Texture>(
                ("/texture/" + spec) / et,
                Constant_Spectrum_Texture{
                    fetch<spectra::Spectrum>(("/spectrum/" + spec) / et),
                }
            );

        MTT_DESERIALIZE_CALLBACK([]{
            auto size = stl::vector<image::Image>::instance().size();
            auto cap = stl::vector<Vector_Texture>::instance().capacity<Image_Vector_Texture>();
            stl::vector<image::Image>::instance().reserve(size + cap);
        }, []{},
        Vector_Texture, Constant_Vector_Texture, Image_Vector_Texture);

        MTT_DESERIALIZE_CALLBACK([]{
            auto size = stl::vector<image::Image>::instance().size();
            auto scap = stl::vector<Spectrum_Texture>::instance().capacity<Image_Spectrum_Texture>();
            auto vcap = stl::vector<Vector_Texture>::instance().capacity<Image_Vector_Texture>();
            stl::vector<Vector_Texture>::instance().reserve<Image_Vector_Texture>(scap + vcap);
            stl::vector<image::Image>::instance().reserve(size + scap);
        }, []{},
        Spectrum_Texture, Constant_Spectrum_Texture, Image_Spectrum_Texture, Checkerboard_Texture);

        MTT_DESERIALIZE(Material, Physical_Material, Interface_Material);
        MTT_DESERIALIZE(Divider);
    }
}
