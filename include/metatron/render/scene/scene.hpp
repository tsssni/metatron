#pragma once
#include <metatron/resource/serde/args.hpp>

namespace mtt::scene {
    auto run(cref<Args> args) noexcept -> void;
}
