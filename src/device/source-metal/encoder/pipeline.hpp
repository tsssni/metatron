#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/pipeline.hpp>

namespace mtt::encoder {
    struct Pipeline_Encoder::Impl final {
        MTL::ComputeCommandEncoder* encoder;
    };
}
