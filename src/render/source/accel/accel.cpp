#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::accel {
    auto Acceleration::init() noexcept -> void {
        MTT_DESERIALIZE(Divider);
        MTT_DESERIALIZE_CALLBACK(
        [](ref<scene::Hierarchy::binmap> bins) noexcept {
            if (!bins.contains("hwbvh")) return;
            bins.erase("hwbvh");
            bins["lbvh"].clear();
            bins["lbvh"].push_back({
                .entity = "/accel",
                .type = "lbvh",
                .serialized = "{}",
            });
            stl::print("fallback to LBVH on CPU");
        }, scene::Hierarchy::default_filter, LBVH, HWBVH);
    }
}
