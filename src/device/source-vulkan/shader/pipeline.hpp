#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        vk::UniqueShaderModule module;
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;
        std::vector<vk::UniqueDescriptorSetLayout> sets;

        Impl(std::string_view shader, std::string_view entry) noexcept;
        auto static global_set() noexcept -> vk::DescriptorSetLayout;
        auto static to_sets(cref<Layout> layout) noexcept -> std::vector<vk::UniqueDescriptorSetLayout>;
    };
}
