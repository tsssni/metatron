#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        vk::UniqueShaderModule module;
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;

        Impl(
            std::string_view name,
            std::vector<view<Argument>> args
        ) noexcept;
    };
}
