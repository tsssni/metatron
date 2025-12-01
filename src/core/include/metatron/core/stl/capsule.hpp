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

            Impl(rref<Impl> impl) noexcept {
                *this = std::move(impl);
            }

            auto operator=(rref<Impl> impl) noexcept {
                if (impl) deleter(impl);
                impl = impl.impl;
                impl.impl = nullptr;
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
