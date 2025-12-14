#include "buffer.hpp"
#include "queue.hpp"

namespace mtt::command {
    Buffer::Buffer() noexcept {}
    Buffer::~Buffer() noexcept {
        for (auto [timeline, count]: signals)
            timeline->wait(count);
    }
}
