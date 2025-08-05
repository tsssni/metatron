#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <glaze/glaze.hpp>
#include <print>

namespace mtt::serde {
	struct json final {
		ecs::Entity entity;
		std::string type;
		glz::raw_json serialized;
	};
}

namespace glz {
	template<>
	struct from<JSON, mtt::ecs::Entity> {
		template<auto Opts>
		auto static op(mtt::ecs::Entity& v, auto&&... args) noexcept -> void {
			auto path = std::string{};
			parse<JSON>::op<Opts>(path, args...);
			v = mtt::ecs::to_entity(path);
		}
	};

	template<>
	struct to<JSON, mtt::ecs::Entity> {
		template<auto Opts>
		auto static op(mtt::ecs::Entity const& v, auto&&... args) noexcept -> void {
			auto path = mtt::ecs::to_path(v);
			serialize<JSON>::op<Opts>(path, args...);
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
		auto static op(mtt::math::Matrix<T, first_dim, rest_dims...> const& v, auto&&... args) noexcept -> void {
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
		auto static op(mtt::math::Quaternion<T> const& v, auto&&... args) noexcept -> void {
			auto const& data = mtt::math::Vector<T, 4>{v};
			serialize<JSON>::op<Opts>(data, args...);
		}
	};
}
