#pragma once
#include <memory>

namespace mtt::inline prelude {
    template<typename T>
    concept has_descriptor = requires {
        typename T::Descriptor;
    };

    template<typename T>
    struct descriptor final {
        using type = T;
    };

    template<typename T>
    requires has_descriptor<T>
    struct descriptor<T> final {
        using type = T::Descriptor;
    };

    template<typename T>
    using descriptor_t = descriptor<T>::type;

    template<typename T>
    requires has_descriptor<T>
    auto make_desc(descriptor_t<T> const& desc) noexcept -> std::unique_ptr<T> {
        return std::make_unique<T>(desc);
    }
}
