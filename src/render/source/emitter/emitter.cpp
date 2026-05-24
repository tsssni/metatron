#include <metatron/render/emitter/emitter.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::emitter {
    auto Emitter::init() noexcept -> void {
        MTT_DESERIALIZE(Uniform_Emitter);
    }
}
