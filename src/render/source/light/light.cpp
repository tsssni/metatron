#include <metatron/render/light/light.hpp>

namespace mtt::render {

auto init() noexcept -> void {
    init_atmosphere();
}

}
