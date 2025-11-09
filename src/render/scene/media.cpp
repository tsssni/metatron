#include <metatron/render/scene/serde.hpp>
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/resource/volume/nanovdb.hpp>
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>

namespace glz {
    template<>
    struct meta<mtt::media::Phase::Function> {
        using enum mtt::media::Phase::Function;
        auto constexpr static value = glz::enumerate(
            henyey_greenstein
        );
    };
}

namespace mtt::scene {
    auto media_init() noexcept -> void {
        using namespace volume;
        using namespace media;

        auto& vvec = stl::vector<Volume>::instance();
        vvec.emplace_type<Uniform_Volume>();
        vvec.emplace_type<Nanovdb_Volume>();

        auto& mvec = stl::vector<Medium>::instance();
        mvec.emplace_type<Homogeneous_Medium>();
        mvec.emplace_type<Heterogeneous_Medium>();
        mvec.emplace_type<Vaccum_Medium>();

        attach<Medium>("/hierarchy/medium/vaccum"_et, Vaccum_Medium{});
        MTT_DESERIALIZE(Volume, Uniform_Volume, Nanovdb_Volume);
        MTT_DESERIALIZE_CALLBACK([]{
            auto size = stl::vector<Volume>::instance().size<Uniform_Volume>();
            auto cap = stl::vector<Medium>::instance().capacity<Heterogeneous_Medium>();
            stl::vector<Volume>::instance().reserve<Uniform_Volume>(size + cap);
        }, []{},
        Medium, Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium);
    }
}
