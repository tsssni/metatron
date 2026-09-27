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
            deleter([](void* impl) {
                delete (typename T::Impl*)impl;
            }) {}

            ~Impl() noexcept {
                if (impl) deleter(impl);
            }

            Impl(Impl&& rhs) noexcept {
                *this = std::move(rhs);
            }

            auto operator=(Impl&& rhs) noexcept -> Impl& {
                if (impl) deleter(impl);
                impl = rhs.impl;
                deleter = std::move(rhs.deleter);
                rhs.impl = nullptr;
                return *this;
            }

            auto operator->() noexcept {
                return (typename T::Impl*)impl;
            }

            auto operator->() const noexcept {
                return (typename T::Impl const*)impl;
            }

            auto operator*() noexcept {
                return *this->operator->();
            }

            auto operator*() const noexcept {
                return *this->operator->();
            }

        private:
            void* impl;
            auto (*deleter)(void*) -> void;
        } impl;
    };
}
