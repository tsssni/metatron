#pragma once
#include <metatron/resource/serde/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace glz {
    template<>
    struct meta<mtt::color::Color_Space::Spectrum_Type> {
        using enum mtt::color::Color_Space::Spectrum_Type;
        auto constexpr static value = glz::enumerate(albedo, unbounded, illuminant);
    };

    template<typename T>
    requires mtt::stl::is_polynomial<T> || mtt::stl::is_proxy<T>
    struct meta<T> {
        auto static constexpr custom_read = true;
        auto static constexpr custom_write = true;
    };

    template<typename T>
    requires mtt::stl::is_polynomial<T> || mtt::stl::is_proxy<T>
    struct from<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto path = std::string{};
            parse<JSON>::op<Opts>(path, args...);
            v = T::entity(path);
        }
    };

    template<typename T>
    requires mtt::has_descriptor<T>
    struct from<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto desc = mtt::descriptor_t<T>{};
            parse<JSON>::op<Opts>(desc, args...);
            v = std::move(desc);
        }
    };

    template<typename T, mtt::usize first_dim, mtt::usize... rest_dims>
    struct from<JSON, mtt::math::Matrix<T, first_dim, rest_dims...>> {
        template<auto Opts>
        auto static op(mtt::math::Matrix<T, first_dim, rest_dims...>& v, auto&&... args) noexcept -> void {
            using M = mtt::math::Matrix<T, first_dim, rest_dims...>;
            using E = M::Element;
            auto data = std::array<E, first_dim>{};
            parse<JSON>::op<Opts>(data, args...);
            v = M{std::span<E const>{data}};
        }
    };

    template<typename T>
    requires std::floating_point<T>
    struct from<JSON, mtt::math::Quaternion<T>> {
        template<auto Opts>
        auto static op(mtt::math::Quaternion<T>& v, auto&&... args) noexcept -> void {
            auto data = mtt::math::Vector<T, 4>{};
            parse<JSON>::op<Opts>(data, args...);
            v = mtt::math::Quaternion<T>{data};
        }
    };
}
