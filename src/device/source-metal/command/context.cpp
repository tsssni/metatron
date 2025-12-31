#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "context.hpp"
#include <metatron/core/stl/print.hpp>

namespace mtt::command {
    Context::Impl::Impl() noexcept {
        device = MTL::CreateSystemDefaultDevice();
    }

    auto Context::init() noexcept -> void {
        Context::instance();
    }
}
