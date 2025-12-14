#include "buffer.hpp"

namespace mtt::opaque {
    Buffer::Buffer(cref<Descriptor> desc) noexcept {}
    Buffer::~Buffer() noexcept {};

    Buffer::operator View() noexcept {
        return {this, 0, size};
    }
}
