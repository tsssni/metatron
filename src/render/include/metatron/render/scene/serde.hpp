#pragma once
#include <metatron/render/scene/entity.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/stl/vector.hpp>
#include <glaze/glaze.hpp>

namespace glz {
    template<typename T>
    concept has_descriptor = requires {
        typename T::Descriptor;
    };

    template<typename T>
    struct descriptor final {
        using type = T;
    };

    template<typename T>
    requires has_descriptor<T>
    struct descriptor<T> final {
        using type = T::Descriptor;
    };

    template<>
    struct from<JSON, mtt::scene::Entity> {
        template<auto Opts>
        auto static op(mtt::scene::Entity& v, auto&&... args) noexcept -> void {
            auto path = std::string{};
            parse<JSON>::op<Opts>(path, args...);
            v = path / mtt::scene::et;
        }
    };

    template<typename F>
    struct from<JSON, mtt::stl::proxy<F>> {
        template<auto Opts>
        auto static op(mtt::stl::proxy<F>& v, auto&&... args) noexcept -> void {
            auto entity = mtt::scene::Entity{};
            parse<JSON>::op<Opts>(entity, args...);
            v = mtt::scene::Hierarchy::instance().fetch<F>(entity);
        }
    };

    template<typename T>
    requires has_descriptor<T>
    struct to<JSON, T> {
        template<auto Opts>
        auto static op(T& v, auto&&... args) noexcept -> void {
            auto desc = (typename descriptor<T>::type){};
            serialize<JSON>::op<Opts>(desc, args...);
            v = desc;
        }
    };

    template<pro::facade F, typename... Ts>
    requires (sizeof...(Ts) > 1)
    struct to<JSON, mtt::stl::vector<F, Ts...>> {
        template<auto Opts>
        auto static op(mtt::stl::vector<F, Ts...>& v, auto&&... args) noexcept -> void {
            auto var = std::variant<typename descriptor<Ts>::type...>{};
            serialize<JSON>::op<Opts>(var, args...);
            std::visit([&v](auto&& desc) {
                ([&v]<typename T>(auto&& d) {
                    if constexpr (std::is_same_v<
                        typename descriptor<T>::type,
                        std::remove_cvref_t<decltype(desc)>
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

    template<typename T, mtt::usize first_dim, mtt::usize... rest_dims>
    struct to<JSON, mtt::math::Matrix<T, first_dim, rest_dims...>> {
        template<auto Opts>
        auto static op(mtt::math::Matrix<T, first_dim, rest_dims...>& v, auto&&... args) noexcept -> void {
            using E = mtt::math::Matrix<T, first_dim, rest_dims...>::Element;
            auto const& data = std::array<E, first_dim>(v);
            serialize<JSON>::op<Opts>(data, args...);
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

    template<typename T>
    requires std::floating_point<T>
    struct to<JSON, mtt::math::Quaternion<T>> {
        template<auto Opts>
        auto static op(mtt::math::Quaternion<T>& v, auto&&... args) noexcept -> void {
            auto const& data = mtt::math::Vector<T, 4>{v};
            serialize<JSON>::op<Opts>(data, args...);
        }
    };
}
