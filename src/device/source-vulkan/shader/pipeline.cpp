#include "pipeline.hpp"
#include "argument.hpp"
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::shader {
    Pipeline::Pipeline(cref<Descriptor> desc) noexcept: args(std::move(desc.args)) {
        auto base_path = stl::path{"shader"} / desc.name;
        auto ir_path = stl::path{base_path}.concat(".spirv");
        auto spirv = stl::filesystem::load(stl::filesystem::find(ir_path), std::ios::binary);

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto cache = ctx->pipeline_cache.get();

        auto entry = desc.name.substr(desc.name.find_last_of(".") + 1);
        auto layouts = args
        | std::views::transform([](auto&& x){ return x->impl->layout.get(); })
        | std::ranges::to<std::vector<vk::DescriptorSetLayout>>();
        impl->sets = args
        | std::views::transform([device](auto&& x){ return x->set->addr; })
        | std::ranges::to<std::vector<vk::DeviceAddress>>();

        impl->module = command::guard(device.createShaderModuleUnique({
            .codeSize = spirv.size(),
            .pCode = view<u32>(spirv.data()),
        }));
        impl->layout = command::guard(device.createPipelineLayoutUnique({
            .setLayoutCount = u32(layouts.size()),
            .pSetLayouts = layouts.data(),
        }));
        impl->pipeline = command::guard(device.createComputePipelineUnique(cache, {
            .flags = vk::PipelineCreateFlagBits::eDescriptorBufferEXT,
            .stage = {
                .stage = vk::ShaderStageFlagBits::eCompute,
                .module = impl->module.get(),
                .pName = entry.data(),
            },
            .layout = impl->layout.get(),
        }));
    }
}
