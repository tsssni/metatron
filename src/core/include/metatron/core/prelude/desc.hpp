#pragma once
#include <metatron/core/prelude/life.hpp>

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
    auto make_desc(cref<typename T::Descriptor> desc) noexcept -> obj<T> {
        return make_obj<T>(desc);
    }
}
