#pragma once
#include <metatron/core/stl/concept.hpp>
#include <metatron/resource/serde/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>

namespace glz {
    template<>
    struct meta<mtt::color::Color_Space::Spectrum_Type> {
        using enum mtt::color::Color_Space::Spectrum_Type;
        auto constexpr static value = glz::enumerate(albedo, unbounded, illuminant);
    };

    template<typename T>
    requires mtt::stl::polymorphic<T> || mtt::stl::proxied<T> || mtt::stl::sealed<T>
    struct meta<T> {
        auto static constexpr custom_read = true;
        auto static constexpr custom_write = true;
    };

    template<typename T>
    requires mtt::stl::polymorphic<T> || mtt::stl::proxied<T> || mtt::stl::sealed<T>
    struct from<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto path = std::string{};
            parse<JSON>::op<Opts>(path, args...);
            if constexpr (mtt::stl::sealed<T>) v = T::vs::entity(path);
            else v = T::entity(path);
        }
    };

    template<typename T>
    requires mtt::described<T>
    struct from<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto desc = mtt::descriptor_t<T>{};
            parse<JSON>::op<Opts>(desc, args...);
            v = std::move(desc);
        }
    };

    template<typename T, mtt::usize dim0, mtt::usize... rest>
    struct from<JSON, mtt::math::matrix<T, dim0, rest...>> {
        template<auto Opts>
        auto static op(mtt::math::matrix<T, dim0, rest...>& v, auto&&... args) noexcept -> void {
            using M = mtt::math::matrix<T, dim0, rest...>;
            using E = M::Element;
            auto data = std::array<E, dim0>{};
            parse<JSON>::op<Opts>(data, args...);
            v = M{std::span<E const>{data}};
        }
    };

    template<typename T>
    requires mtt::stl::floating_point<T>
    struct from<JSON, mtt::math::quaternion<T>> {
        template<auto Opts>
        auto static op(mtt::math::quaternion<T>& v, auto&&... args) noexcept -> void {
            auto data = mtt::math::vector<T, 4>{};
            parse<JSON>::op<Opts>(data, args...);
            v = mtt::math::quaternion<T>{data};
        }
    };
}
