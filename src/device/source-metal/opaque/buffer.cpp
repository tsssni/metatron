#include "buffer.hpp"

namespace mtt::opaque {
    Buffer::Buffer(cref<Descriptor> desc) noexcept {}
    Buffer::~Buffer() noexcept {};
    auto Buffer::upload() noexcept -> obj<Buffer> { return {}; };
    auto Buffer::flush(view<Buffer> dst, usize offset) noexcept -> obj<Buffer> { return {}; }
}
