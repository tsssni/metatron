#pragma once

namespace mtt::scene {
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
}
