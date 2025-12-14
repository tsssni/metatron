#include "timeline.hpp"

namespace mtt::command {
    Timeline::Timeline() noexcept {}
    auto Timeline::wait(u64 count, u64 timeout) noexcept -> bool { return false; }
    auto Timeline::signal(u64 count) noexcept -> void {}
}
