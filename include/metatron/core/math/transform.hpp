#pragma once
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/stl/protocol.hpp>
#include <metatron/core/stl/print.hpp>
#include <array>

namespace mtt::math {
    template<typename T>
    concept Transformable = false
    || std::same_as<std::remove_cvref_t<T>, fv4>
    || std::same_as<std::remove_cvref_t<T>, fv3> // for normal
    || std::same_as<std::remove_cvref_t<T>, Ray>
    || std::same_as<std::remove_cvref_t<T>, Ray_Differential>;

    struct Transform final {
        struct Chain final {
            auto operator|(cref<Transform> t) noexcept -> ref<Chain> {
                set(0);
                store(t);
                return *this;
            }

            auto operator^(cref<Transform> t) noexcept -> ref<Chain> {
                set(1);
                store(t);
                return *this;
            }

            template<Transformable T>
            auto operator|(cref<T> t) {
                set(0);
                return dechain(t);
            }

            template<Transformable T>
            auto operator^(cref<T> t) {
                set(1);
                return dechain(t);
            }

            explicit operator fm44() const {
                auto& ts = *transforms;
                auto ret = ts[size - 1]->transform;
                for (auto i = i32(size) - 2; i >= 0; i--)
                    ret = (((ops >> i) & 1) == 0)
                    ? ts[i]->transform | ret
                    : ts[i]->inv_transform | ret;
                return ret;
            }

        private:
            Chain(cref<Transform> t) {
                store(t);
            }

            auto store(cref<Transform> t) noexcept -> void {
                if (size >= max_transforms) [[unlikely]]
                    stl::abort("transform chain overflow");
                (*transforms)[size++] = &t;
            }

            auto set(byte v) noexcept -> void {
                ops |= byte(v << (size - 1));
            }

            template<Transformable T, typename Type = std::remove_cvref_t<T>>
            auto dechain(cref<T> rhs) noexcept -> Type {
                auto& ts = *transforms;
                auto ret = rhs;
                for (auto i = i32(size) - 1; i >= 0; i--)
                    ret = (((ops >> i) & 1) == 0)
                    ? *ts[i] | ret
                    : *ts[i] ^ ret;
                return ret;
            }

            auto constexpr static max_transforms = 4;
            using Array = std::array<view<Transform>, max_transforms>;
            std::unique_ptr<Array> transforms = std::make_unique<Array>();
            byte ops{};
            byte size{};

            friend Transform;
        };

        fm44 transform{1.f};
        fm44 inv_transform{1.f};

        Transform() noexcept = default;
        Transform(std::initializer_list<math::Transform> list) {
            for (auto const& t: list)
                transform = transform | t.transform;
            for (auto const& t: std::views::reverse(list))
                inv_transform = inv_transform | t.inv_transform;
        }

        explicit Transform(cref<fm44> m):
        transform(m), inv_transform(math::inverse(m)) {}

        explicit operator fm44() const {
            return transform;
        }

        template<Transformable T, typename Type = std::remove_cvref_t<T>>
        auto operator|(T&& rhs) const {
            if constexpr (std::same_as<Type, fv4>) {
                return transform | rhs;
            } else if constexpr (std::same_as<Type, fv3>) {
                return math::normalize(shrink(expand(rhs, 0.f) | inv_transform));
            } else if constexpr (std::same_as<Type, Ray>) {
                auto r = rhs;
                r.o = *this | expand(r.o, 1.f);
                r.d = *this | expand(r.d, 0.f);
                return r;
            } else if constexpr (std::same_as<Type, Ray_Differential>) {
                auto ray = rhs;
                ray.r = *this | rhs.r;
                ray.rx = *this | rhs.rx;
                ray.ry = *this | rhs.ry;
                return ray;
            }
        }

        auto operator|(cref<Transform> rhs) const noexcept -> Chain {
            return std::move(Chain{*this} | rhs);
        }

        template<Transformable T, typename Type = std::remove_cvref_t<T>>
        auto operator^(T&& rhs) const {
            if constexpr (std::same_as<Type, fv4>) {
                return inv_transform | rhs;
            } else if constexpr (std::same_as<Type, fv3>) {
                return math::normalize(shrink(expand(rhs, 0.f) | transform));
            } else if constexpr (std::same_as<Type, Ray>) {
                auto r = rhs;
                r.o = *this ^ expand(r.o, 1.f);
                r.d = *this ^ expand(r.d, 0.f);
                return r;
            } else if constexpr (std::same_as<Type, Ray_Differential>) {
                auto ray = rhs;
                ray.r = *this ^ rhs.r;
                ray.rx = *this ^ rhs.rx;
                ray.ry = *this ^ rhs.ry;
                return ray;
            }
        }

        template<typename T>
        requires std::same_as<std::remove_cvref_t<T>, Transform>
        auto operator^(T&& rhs) const noexcept -> Chain {
            return std::move(Chain{*this} ^ rhs);
        }
    };

    auto inline inverse(cref<math::Transform> t) -> math::Transform {
        auto inv_t = math::Transform{};
        inv_t.transform = t.inv_transform;
        inv_t.inv_transform = t.transform;
        return inv_t;
    }
}

namespace mtt::math::proxy {
    struct Transform: stl::proxy<Transform, math::Transform> {
        using proxy::proxy;
        explicit operator fm44() const noexcept { return (fm44)(*idx); }

        template<math::Transformable T>
        auto operator|(T&& rhs) const { return (*idx) | std::forward<T>(rhs); }
        template<math::Transformable T>
        auto operator^(T&& rhs) const { return (*idx) ^ std::forward<T>(rhs); }
        auto operator|(cref<math::Transform> rhs) const noexcept { return (*idx) | rhs; }
        auto operator^(cref<math::Transform> rhs) const noexcept { return (*idx) ^ rhs; }
        auto operator|(Transform rhs) const noexcept { return (*idx) | (cref<math::Transform>)rhs; }
        auto operator^(Transform rhs) const noexcept { return (*idx) ^ (cref<math::Transform>)rhs; }
    };
}
