#include "pipeline.hpp"
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::shader {
    Pipeline::Impl::Impl(
        std::string_view shader,
        std::string_view entry
    ) noexcept {
        auto base_path = stl::path{std::format("shader/{}.{}", shader, entry)};
        auto ir_path = stl::path{base_path}.concat(".spirv");
        auto table_path = stl::path{base_path}.concat(".json");
        auto spirv = stl::filesystem::load(stl::filesystem::find(ir_path));
        auto table = Layout{};
        stl::json::load(stl::filesystem::find(table_path), table);

        auto& ctx = command::Context::instance();
        auto& device = ctx.device;
        auto& cache = ctx.pipeline_cache;

        sets = to_sets(table);
        auto layouts = std::vector<vk::DescriptorSetLayout>{global_set()};
        std::ranges::copy(
            std::views::transform(sets, [](auto&& x){ return x.get(); }),
            std::back_inserter(layouts)
        );

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

    // solve the problem of destroy order
    auto Pipeline::Impl::global_set() noexcept -> vk::DescriptorSetLayout {
        auto static set = vk::UniqueDescriptorSetLayout{};
        if (set) return set.get();
        auto layout = shader::Layout{};
        stl::json::load(stl::filesystem::find("shader/metatron.json"), layout);
        auto layouts = to_sets(layout);
        if (layouts.size() != 1) {
            std::println("no valid vulkan global pipeline layout");
            std::abort();
        }
        set = std::move(layouts.front());
        return set.get();
    }

    auto Pipeline::Impl::to_sets(cref<Layout> layout) noexcept -> std::vector<vk::UniqueDescriptorSetLayout> {
        auto sets = std::vector<vk::UniqueDescriptorSetLayout>{};
        for (auto& s: layout.sets) {
            auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};
            for (auto i = 0u; i < s.size(); ++i) {
                using Type = Layout::Descriptor::Type;
                using Access = Layout::Descriptor::Access;
                using Vk_Type = vk::DescriptorType;
                auto constexpr types = std::to_array<vk::DescriptorType>({
                    Vk_Type::eUniformBuffer,
                    Vk_Type::eSampler,
                    Vk_Type::eSampledImage,
                    Vk_Type::eSampledImage,
                    Vk_Type::eAccelerationStructureKHR,
                });

                auto& desc = s[i];
                auto type = types[i32(desc.type)];
                if (type == Vk_Type::eSampledImage && desc.access != Access::readonly)
                    type = Vk_Type::eStorageImage;
                auto count = desc.size < 0 ? 16384u : math::max(1, desc.size);
                bindings.push_back(vk::DescriptorSetLayoutBinding{
                    .binding = i,
                    .descriptorType = type,
                    .descriptorCount = count,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute,
                });
            }

            auto device = command::Context::instance().device;
            sets.push_back(command::guard(device.createDescriptorSetLayoutUnique({
                .bindingCount = u32(bindings.size()),
                .pBindings = bindings.data(),
            })));
        }
        return sets;
    }

    Pipeline::Pipeline(
        std::string_view shader,
        std::string_view entry
    ) noexcept: stl::capsule<Pipeline>(shader, entry) {}
}
