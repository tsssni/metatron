#include "queue.hpp"

namespace mtt::command {
    Queue::Queue(Queue::Type type) noexcept {}
    Queue::~Queue() noexcept {}
    auto Queue::allocate() noexcept -> obj<Buffer> { return {}; }
    auto Queue::submit(rref<obj<Buffer>> cmd) noexcept -> void {}
}
