#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        std::vector<byte> spirv;
        Layout layout;

        Impl(
            std::string_view shader,
            std::string_view entry
        ) noexcept {
            auto base_path = std::format("{}.{}", shader, entry);
            auto spirv_path = "shader/" + base_path + ".spirv";
            auto json_path = "shader/" + base_path + ".json";

            auto callback = [](std::string_view path) {
                std::println("pipeline {} not found", path);
                std::abort();
            };
            auto& fs = stl::filesystem::instance();
            spirv = fs.load(fs.find(spirv_path));
            stl::json::load(fs.find(json_path), layout);
        }
    };

    Pipeline::Pipeline(
        std::string_view shader,
        std::string_view entry
    ) noexcept: stl::capsule<Pipeline>(shader, entry) {}
}
