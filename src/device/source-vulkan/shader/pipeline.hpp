#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        vk::UniqueShaderModule module;
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;
        std::vector<vk::DeviceAddress> sets;
    };
}
