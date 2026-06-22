#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/serde/serde.hpp>
#include <metatron/resource/serde/args.hpp>

namespace mtt::accel {
    auto Acceleration::init() noexcept -> void {
        MTT_DESERIALIZE(Divider);
        MTT_DESERIALIZE_CALLBACK(
        [](ref<scene::Hierarchy::binmap> bins) noexcept {
            auto gpu = scene::Args::instance().device == "gpu";
            auto reset = [&bins](auto a, auto b) {
                bins.erase(b);
                bins[a].clear();
                bins[a].push_back({
                    .entity = "/accel",
                    .type = a,
                    .serialized = "{}",
                });
            };
            if (bins.contains("hwbvh") && !gpu) {
                reset("lbvh", "hwbvh");
                stl::print("fallback to LBVH on CPU");
            } else if (!bins.contains("hwbvh") && gpu) {
                reset("hwbvh", "lbvh");
                stl::print("fallback to HWBVH on GPU");
            }
        }, scene::Hierarchy::default_filter, LBVH, HWBVH);
    }
}
