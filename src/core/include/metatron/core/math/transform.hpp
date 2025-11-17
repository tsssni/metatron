#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <vector>

namespace mtt::math {
    template<typename T>
    concept Transformable = false
    || std::is_same_v<std::remove_cvref_t<T>, fv4>
    || std::is_same_v<std::remove_cvref_t<T>, fv3> // for normal
    || std::is_same_v<std::remove_cvref_t<T>, Ray>
    || std::is_same_v<std::remove_cvref_t<T>, Ray_Differential>;

    struct Transform final {
        struct Chain final {
            auto operator|(cref<Transform> t) noexcept -> ref<Chain> {
                store(t);
                ops.push_back(0);
                return *this;
            }

            auto operator^(cref<Transform> t) noexcept -> ref<Chain> {
                store(t);
                ops.push_back(1);
                return *this;
            }

            template<Transformable T>
            auto operator|(cref<T> t) {
                ops.push_back(0);
                return dechain(t);
            }

            template<Transformable T>
            auto operator^(cref<T> t) {
                ops.push_back(1);
                return dechain(t);
            }

            explicit operator fm44() const {
                auto ret = transforms.back()->transform;
                for (auto i = i32(transforms.size()) - 2; i >= 0; i--)
                    ret = (ops[i] == 0)
                    ? transforms[i]->transform | ret
                    : transforms[i]->inv_transform | ret;
                return ret;
            }

        private:
            Chain(cref<Transform> t) {
                store(t);
            }

            auto store(cref<Transform> t) noexcept -> void {
                transforms.push_back(&t);
            }

            template<Transformable T, typename Type = std::remove_cvref_t<T>>
            auto dechain(cref<T> rhs) noexcept -> Type {
                auto ret = rhs;
                for (auto i = i32(transforms.size()) - 1; i >= 0; i--)
                    ret = (ops[i] == 0)
                    ? *transforms[i] | ret
                    : *transforms[i] ^ ret;
                return ret;
            }

            std::vector<view<Transform>> transforms;
            std::vector<Transform> owned_transforms;
            std::vector<byte> ops;

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

        explicit Transform(cref<fm44> m)
        : transform(m), inv_transform(math::inverse(m)) {}

        explicit operator fm44() const {
            return transform;
        }

        template<Transformable T, typename Type = std::remove_cvref_t<T>>
        auto operator|(T&& rhs) const {
            if constexpr (std::is_same_v<Type, fv4>) {
                return transform | rhs;
            } else if constexpr (std::is_same_v<Type, fv3>) {
                return math::normalize(shrink(expand(rhs, 0.f) | inv_transform));
            } else if constexpr (std::is_same_v<Type, Ray>) {
                auto r = rhs;
                r.o = *this | expand(r.o, 1.f);
                r.d = *this | expand(r.d, 0.f);
                return r;
            } else if constexpr (std::is_same_v<Type, Ray_Differential>) {
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
            if constexpr (std::is_same_v<Type, fv4>) {
                return inv_transform | rhs;
            } else if constexpr (std::is_same_v<Type, fv3>) {
                return math::normalize(shrink(expand(rhs, 0.f) | transform));
            } else if constexpr (std::is_same_v<Type, Ray>) {
                auto r = rhs;
                r.o = *this ^ expand(r.o, 1.f);
                r.d = *this ^ expand(r.d, 0.f);
                return r;
            } else if constexpr (std::is_same_v<Type, Ray_Differential>) {
                auto ray = rhs;
                ray.r = *this ^ rhs.r;
                ray.rx = *this ^ rhs.rx;
                ray.ry = *this ^ rhs.ry;
                return ray;
            }
        }

        template<typename T>
        requires std::is_same_v<std::remove_cvref_t<T>, Transform>
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
