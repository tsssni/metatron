#pragma once
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/print.hpp>

namespace glz {
    template<>
    struct meta<mtt::spectra::Color_Space::Spectrum_Type> {
        using enum mtt::spectra::Color_Space::Spectrum_Type;
        auto constexpr static value = glz::enumerate(albedo, unbounded, illuminant);
    };

    template<typename F>
    struct from<JSON, mtt::tag<F>> {
        template<auto Opts>
        auto static op(mtt::tag<F>& v, auto&&... args) noexcept -> void {
            auto path = std::string{};
            parse<JSON>::op<Opts>(path, args...);
            v = mtt::entity<F>(path);
        }
    };

    template<typename T>
    requires mtt::has_descriptor<T>
    struct from<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto desc = (typename mtt::descriptor<T>::type){};
            parse<JSON>::op<Opts>(desc, args...);
            v = std::move(desc);
        }
    };

    template<pro::facade F, typename... Ts> struct from<JSON, mtt::stl::variant<F, Ts...>> {
        template<auto Opts>
        auto static op(mtt::stl::variant<F, Ts...>& v, auto&&... args) noexcept -> void {
            auto var = std::variant<typename mtt::descriptor<Ts>::type...>{};
            parse<JSON>::op<Opts>(var, args...);
            std::visit([&v](auto&& desc) {
                ([&v]<typename T>(auto&& d) {
                    if constexpr (std::is_same_v<
                        typename mtt::descriptor<T>::type,
                        std::decay_t<decltype(desc)>
                    >) v.template emplace<T>(std::forward<decltype(d)>(d));
                }.template operator()<Ts>(desc), ...);
            }, var);
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
