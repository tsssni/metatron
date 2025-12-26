#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/pipeline.hpp>

namespace mtt::encoder {
    struct Pipeline_Encoder::Impl final {
        opaque::Barrier barrier;
    };
}
