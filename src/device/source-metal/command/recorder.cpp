#include "recorder.hpp"

namespace mtt::command {
    Retention::Retention() noexcept {}
    Retentions::~Retentions() noexcept {}
    Recorder::Recorder(Queue::Type type)noexcept: type(type) {}
    Recorder::~Recorder() noexcept {}
}
