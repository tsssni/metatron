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
        auto constexpr static value = glz::enumerate(henyey_greenstein);
    };
}

namespace mtt::scene {
    auto media_init() noexcept -> void {
        using namespace volume;
        using namespace media;
        MTT_DESERIALIZE(Volume, Uniform_Volume, Nanovdb_Volume);
        MTT_DESERIALIZE(Medium, Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium);
        stl::vector<Medium>::instance().emplace<Vaccum_Medium>("/medium/vaccum");
    }
}
