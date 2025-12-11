#include "buffer.hpp"

namespace mtt::command {
    Retention::Retention() noexcept {}
    Retentions::~Retentions() noexcept {}
    Buffer::Buffer(Queue::Type type)noexcept: type(type) {}
    Buffer::~Buffer() noexcept {}
}
