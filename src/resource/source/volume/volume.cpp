#include <metatron/resource/volume/volume.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::volume {
    auto Volume::init() noexcept -> void {
        MTT_DESERIALIZE(Uniform_Volume, Nanovdb_Volume);
        stl::vector<muldim::Grid>::init();
        stl::vector<Nanovdb_Volume::Grid>::init();
    }
}
