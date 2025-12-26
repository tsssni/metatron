#include "image.hpp"

namespace mtt::opaque {
    Image::Image(cref<Descriptor> desc) noexcept {}

    Image::operator View() noexcept {
        return {this, {0, mips}, {0, 0}, {width, height}};
    }
}
