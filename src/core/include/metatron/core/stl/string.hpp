#include <metatron/core/stl/hash.hpp>
#include <string_view>

namespace mtt::stl {
    template<>
    struct hash<std::string> final {
        using hasher = std::hash<std::string_view>;
        using is_transparent = void;
        auto operator()(view<char> str) const { return hasher{}(str); }
        auto operator()(cref<std::string> str) const { return hasher{}(str); }
        auto operator()(std::string_view str) const { return hasher{}(str); }
    };
}
