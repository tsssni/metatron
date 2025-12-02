#include "pipeline.hpp"
#include "argument.hpp"
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::shader {
    Pipeline::Impl::Impl(
        std::string_view name,
        std::vector<view<Argument>> args
    ) noexcept {
        auto base_path = stl::path{"shader"} / name;
        auto ir_path = stl::path{base_path}.concat(".spirv");
        auto table_path = stl::path{base_path}.concat(".json");
        auto spirv = stl::filesystem::load(stl::filesystem::find(ir_path));

        auto& ctx = command::Context::instance();
        auto& device = ctx.device;
        auto& cache = ctx.pipeline_cache;

        auto entry = name.substr(name.find_last_of(".") + 1);
        auto layouts = args
        | std::views::transform([](auto&& x){ return x->impl->layout.get(); })
        | std::ranges::to<std::vector<vk::DescriptorSetLayout>>();

        module = command::guard(device.createShaderModuleUnique({
            .codeSize = spirv.size(),
            .pCode = view<u32>(spirv.data()),
        }));
        layout = command::guard(device.createPipelineLayoutUnique({
            .setLayoutCount = u32(layouts.size()),
            .pSetLayouts = layouts.data(),
        }));
        pipeline = command::guard(device.createComputePipelineUnique(cache, {
            .stage = {
                .stage = vk::ShaderStageFlagBits::eCompute,
                .module = module.get(),
                .pName = entry.data(),
            },
            .layout = layout.get(),
        }));
    }

    Pipeline::Pipeline(
        std::string_view shader,
        std::vector<view<Argument>> args
    ) noexcept: stl::capsule<Pipeline>(shader, args) {}
}
