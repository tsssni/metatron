#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        mut<MTL::BlitCommandEncoder> encoder;
    };
}
