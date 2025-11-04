#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/resource/volume/nanovdb.hpp>
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>

namespace mtt::scene {
    auto media_init() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto& vvec = stl::vector<volume::Volume>::instance();
        vvec.emplace_type<volume::Uniform_Volume>();
        vvec.emplace_type<volume::Nanovdb_Volume>();

        auto& mvec = stl::vector<media::Medium>::instance();
        mvec.emplace_type<media::Homogeneous_Medium>();
        mvec.emplace_type<media::Heterogeneous_Medium>();
        mvec.emplace_type<media::Vaccum_Medium>();
    }
}
