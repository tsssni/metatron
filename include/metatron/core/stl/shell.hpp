#pragma once
#include <functional>

namespace mtt::core {

template<typename T>
struct shell {
    template<typename... Args>
    shell(Args&&... args) noexcept;

    struct implementation final {
        template<typename... Args>
        implementation(Args&&... args) noexcept;
        ~implementation() noexcept;
        implementation(implementation&& rhs) noexcept;

        auto operator=(implementation&& rhs) noexcept -> implementation&;
        auto operator->() noexcept;
        auto operator->() const noexcept;
        auto operator*() noexcept;
        auto operator*() const noexcept;

    private:
        void* impl;
        auto (*deleter)(void*) -> void;
    } impl;
};

template<typename T>
template<typename... Args>
shell<T>::shell(Args&&... args) noexcept
: impl(std::forward<Args>(args)...) {}

template<typename T>
template<typename... Args>
shell<T>::implementation::implementation(Args&&... args) noexcept
: impl(new typename T::contents(std::forward<Args>(args)...)), deleter([](void* impl) { delete (typename T::contents*)impl; }) {}

template<typename T>
shell<T>::implementation::~implementation() noexcept {
    if (impl) deleter(impl);
}

template<typename T>
shell<T>::implementation::implementation(implementation&& rhs) noexcept {
    *this = std::move(rhs);
}

template<typename T>
auto shell<T>::implementation::operator=(implementation&& rhs) noexcept -> implementation& {
    if (impl) deleter(impl);
    impl = rhs.impl;
    deleter = std::move(rhs.deleter);
    rhs.impl = nullptr;
    return *this;
}

template<typename T>
auto shell<T>::implementation::operator->() noexcept {
    return (typename T::contents*)impl;
}

template<typename T>
auto shell<T>::implementation::operator->() const noexcept {
    return (typename T::contents const*)impl;
}

template<typename T>
auto shell<T>::implementation::operator*() noexcept {
    return *this->operator->();
}

template<typename T>
auto shell<T>::implementation::operator*() const noexcept {
    return *this->operator->();
}

}
