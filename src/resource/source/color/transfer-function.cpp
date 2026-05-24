#include <metatron/resource/color/transfer-function.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::color {
    auto Transfer_Function::init() noexcept -> void {
        MTT_DESERIALIZE(Rec709_Transfer_Function);
        Transfer_Function::vs::instance().emplace<Rec709_Transfer_Function>("/transfer-function/Rec709");
    }
}
