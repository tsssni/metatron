#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/device/shader/layout.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        mtl<MTL::Library> library;
        mtl<MTL::Function> function;
        mtl<MTL::ComputePipelineState> pipeline;
    };
}
