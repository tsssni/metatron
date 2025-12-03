#pragma once
#include <functional>

namespace mtt::stl {
    template<typename T>
    struct capsule {
        template<typename... Args>
        capsule(Args&&... args)
        noexcept: impl(std::forward<Args>(args)...) {}

        struct Impl final {
            template<typename... Args>
            Impl(Args&&... args) noexcept:
            impl(new typename T::Impl(std::forward<Args>(args)...)),
            deleter([](mut<void> impl) {
                delete mut<typename T::Impl>(impl);
            }) {}

            ~Impl() noexcept {
                if (impl) deleter(impl);
            }

            Impl(rref<Impl> rhs) noexcept {
                *this = std::move(rhs);
            }

            auto operator=(rref<Impl> rhs) noexcept {
                if (impl) deleter(impl);
                impl = rhs.impl;
                rhs.impl = nullptr;
                return *this;
            }

            auto operator->() noexcept {
                return mut<typename T::Impl>(impl);
            }

            auto operator->() const noexcept {
                return view<typename T::Impl>(impl);
            }

            auto operator*() noexcept {
                return *this->operator->();
            }

            auto operator*() const noexcept {
                return *this->operator->();
            }

        private:
            mut<void> impl;
            std::function<void(mut<void>)> deleter;
        } impl;
    };
}
