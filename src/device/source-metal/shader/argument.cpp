#include "argument.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {}
    auto Argument::index(std::string_view field) noexcept -> u32 { return 0; }
}
