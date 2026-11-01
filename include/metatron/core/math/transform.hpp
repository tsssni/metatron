#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/stl/ranges.hpp>

namespace mtt::core {

template<typename T>
concept Transformable = false
    || std::same_as<std::remove_cvref_t<T>, fv4>
    || std::same_as<std::remove_cvref_t<T>, fv3> // for normal
    || std::same_as<std::remove_cvref_t<T>, Ray>
    || std::same_as<std::remove_cvref_t<T>, Ray_Differential>;

struct Transform final {
    fm44 transform{1.f};
    fm44 inv_transform{1.f};

    Transform() noexcept = default;
    Transform(std::initializer_list<Transform> list);
    explicit Transform(fm44 const& m);

    explicit operator fm44() const;
    template<Transformable T, typename Type = std::remove_cvref_t<T>>
    auto operator|(T&& rhs) const;
    template<Transformable T, typename Type = std::remove_cvref_t<T>>
    auto operator^(T&& rhs) const;
};

inline Transform::Transform(std::initializer_list<Transform> list) {
    for (auto const& t : list)
        transform = transform | t.transform;
    for (auto const& t : std::views::reverse(list))
        inv_transform = inv_transform | t.inv_transform;
}

inline Transform::Transform(fm44 const& m)
: transform(m), inv_transform(inverse(m)) {}

inline Transform::operator fm44() const { return transform; }

template<Transformable T, typename Type>
auto Transform::operator|(T&& rhs) const {
    if constexpr (std::same_as<Type, fv4>) {
        return transform | rhs;
    } else if constexpr (std::same_as<Type, fv3>) {
        return normalize(shrink(expand(rhs, 0.f) | inv_transform));
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

template<Transformable T, typename Type>
auto Transform::operator^(T&& rhs) const {
    if constexpr (std::same_as<Type, fv4>) {
        return inv_transform | rhs;
    } else if constexpr (std::same_as<Type, fv3>) {
        return normalize(shrink(expand(rhs, 0.f) | transform));
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

auto inline inverse(Transform const& t) -> Transform {
    auto inv_t = Transform{};
    inv_t.transform = t.inv_transform;
    inv_t.inv_transform = t.transform;
    return inv_t;
}

auto constexpr operator|(Transform const& t, Ray_Context const& ctx) -> Ray_Context {
    auto result = ctx;
    result.r = t | result.r;
    result.n = t | result.n;
    return result;
}

auto constexpr operator^(Transform const& t, Ray_Context const& ctx) -> Ray_Context {
    auto result = ctx;
    result.r = t ^ result.r;
    result.n = t ^ result.n;
    return result;
}

}
