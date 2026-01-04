#pragma once
#include "context.hpp"
#include <metatron/device/command/timeline.hpp>

namespace mtt::command {
    struct Timeline::Impl final {
        mtl<MTL::Event> event;
        mtl<MTL::SharedEvent> shared;
    };
}
