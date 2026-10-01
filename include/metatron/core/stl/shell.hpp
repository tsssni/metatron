#pragma once
#include <functional>

namespace mtt::stl {
    template<typename T>
    struct shell {
        template<typename... Args>
        shell(Args&&... args)
        noexcept: impl(std::forward<Args>(args)...) {}

        struct implementation final {
            template<typename... Args>
            implementation(Args&&... args) noexcept:
            impl(new typename T::contents(std::forward<Args>(args)...)),
            deleter([](void* impl) {
                delete (typename T::contents*)impl;
            }) {}

            ~implementation() noexcept {
                if (impl) deleter(impl);
            }

            implementation(implementation&& rhs) noexcept {
                *this = std::move(rhs);
            }

            auto operator=(implementation&& rhs) noexcept -> implementation& {
                if (impl) deleter(impl);
                impl = rhs.impl;
                deleter = std::move(rhs.deleter);
                rhs.impl = nullptr;
                return *this;
            }

            auto operator->() noexcept {
                return (typename T::contents*)impl;
            }

            auto operator->() const noexcept {
                return (typename T::contents const*)impl;
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
