#pragma once
#include "../command/context.hpp"
#include <metatron/device/shader/pipeline.hpp>

namespace mtt::shader {
    struct Pipeline::Impl final {
        mtl<MTL::Library> library;
        mtl<MTL::Function> function;
        mtl<MTL::ComputePipelineState> pipeline;
    };
}
