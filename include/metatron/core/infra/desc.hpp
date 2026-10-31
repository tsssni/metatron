#pragma once
#include <memory>

namespace mtt {

template<typename T>
concept described = requires {
    typename T::Descriptor;
};

template<typename T>
struct descriptor final {
    using type = T;
};

template<typename T>
requires described<T>
struct descriptor<T> final {
    using type = T::Descriptor;
};

template<typename T>
using descriptor_t = descriptor<T>::type;

template<typename T>
auto make_desc(descriptor_t<T> const& desc) noexcept -> std::unique_ptr<T> requires described<T> {
    return std::make_unique<T>(desc);
}

}
