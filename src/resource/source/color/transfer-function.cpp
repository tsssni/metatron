#include <metatron/resource/serde/serde.hpp>

namespace mtt::color {
    auto transfer_function::init() noexcept -> void {
        MTT_DESERIALIZE(Rec709_Transfer_Function);
        Transfer_Function::vs::emplace<Rec709_Transfer_Function>("/transfer-function/Rec709");
    }
}
