#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::math {
	template<typename T>
    concept Transformable = false
	|| std::is_same_v<std::remove_cvref_t<T>, Vector<f32, 4>>
	|| std::is_same_v<std::remove_cvref_t<T>, Ray>
	|| std::is_same_v<std::remove_cvref_t<T>, Ray_Differential>
	|| std::is_same_v<std::remove_cvref_t<T>, eval::Context>;

	struct Transform final {
		struct Config final {
			Vector<f32, 3> translation{};
			Vector<f32, 3> scaling{1.f};
			Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

			auto operator<=>(Config const& rhs) const = default;
		} config;

		struct Chain final {
			template<typename T>
			requires std::is_same_v<std::remove_cvref_t<T>, Transform>
			auto operator|(T&& t) -> Chain& {
				store(std::forward<T>(t));
				ops.push_back(0);
				return *this;
			}

			template<typename T>
			requires std::is_same_v<std::remove_cvref_t<T>, Transform>
			auto operator^(T&& t) -> Chain& {
				store(std::forward<T>(t));
				ops.push_back(1);
				return *this;
			}

			template<Transformable T>
			auto operator|(T&& t) {
				ops.push_back(0);
				return dechain(std::forward<T>(t));
			}

			template<Transformable T>
			auto operator^(T&& t) {
				ops.push_back(1);
				return dechain(std::forward<T>(t));
			}

			explicit operator Matrix<f32, 4, 4>() const {
				auto ret = transforms.back()->transform;
				for (auto i = i32(transforms.size()) - 2; i >= 0; i--) {
					if (ops[i] == 0) {
						ret = transforms[i]->transform | ret;
					} else {
						ret = transforms[i]->inv_transform | ret;
					}
				}
				return ret;
			}

		private:
			template<typename T>
			requires std::is_same_v<std::remove_cvref_t<T>, Transform>
			Chain(T&& t) {
				store(std::forward<T>(t));
			}

			template<typename T>
			requires std::is_same_v<std::remove_cvref_t<T>, Transform>
			auto store(T&& t) {
				if constexpr (std::is_lvalue_reference_v<T>) {
					transforms.push_back(&t);
				} else {
					owned_transforms.push_back(std::forward<T>(t));
					transforms.push_back(&owned_transforms.back());
				}
			}

			template<Transformable T, typename Type = std::remove_cvref_t<T>>
			auto dechain(T&& rhs) -> Type {
				auto ret = rhs;
				for (auto i = i32(transforms.size()) - 1; i >= 0; i--) {
					if (ops[i] == 0) {
						ret = *transforms[i] | ret;
					} else {
						ret = *transforms[i] ^ ret;
					}
				}
				return ret;
			}

			std::vector<Transform const*> transforms;
			std::vector<Transform> owned_transforms;
			std::vector<byte> ops;

			friend Transform;
		};

		mutable Matrix<f32, 4, 4> transform;
		mutable Matrix<f32, 4, 4> inv_transform;

		Transform(
			Vector<f32, 3> translation = {},
			Vector<f32, 3> scaling = {1.f},
			Quaternion<f32> rotation = {0.f, 0.f, 0.f, 1.f}
		): config(translation, scaling, rotation) {
			update();
		}

		auto update() const -> void {
			auto translation = math::Matrix<f32, 4, 4>{
				{1.f, 0.f, 0.f, config.translation[0]},
				{0.f, 1.f, 0.f, config.translation[1]},
				{0.f, 0.f, 1.f, config.translation[2]},
				{0.f, 0.f, 0.f, 1.f}
			};
			auto scaling = math::Matrix<f32, 4, 4>{
				config.scaling[0], config.scaling[1], config.scaling[2], 1.f
			};
			auto rotation = math::Matrix<f32, 4, 4>{config.rotation};

			transform = translation | rotation | scaling;
			inv_transform = math::inverse(transform);
		}

		explicit operator Matrix<f32, 4, 4>() const {
			return transform;
		}

		template<Transformable T, typename Type = std::remove_cvref_t<T>>
		auto operator|(T&& rhs) const {
			if constexpr (std::is_same_v<Type, Vector<f32, 4>>) {
				return transform | rhs;
			} else if constexpr (std::is_same_v<Type, Ray_Differential>) {
				auto ray = rhs;
				ray.r = *this | rhs.r;
				ray.rx = *this | rhs.rx;
				ray.ry = *this | rhs.ry;
				return ray;
			} else if constexpr (std::is_same_v<Type, eval::Context>) {
				auto ctx = rhs;
				ctx.r = *this | ctx.r;
				ctx.n = expand(ctx.n, 0.f) | transform;
				return ctx;
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
			} else if constexpr (std::is_same_v<Type, eval::Context>) {
				auto ctx = rhs;
				ctx.r = *this | ctx.r;
				ctx.n = expand(ctx.n, 0.f) | inv_transform;
				return ctx;
			}
		}

		template<typename T>
		requires std::is_same_v<std::remove_cvref_t<T>, Transform>
		auto operator|(T&& rhs) const -> Chain {
			return Chain{*this} | rhs;
		}

		template<Transformable T, typename Type = std::remove_cvref_t<T>>
		auto operator^(T&& rhs) const {
			if constexpr (std::is_same_v<Type, Vector<f32, 4>>) {
				return inv_transform | rhs;
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
			} else if constexpr (std::is_same_v<Type, eval::Context>) {
				auto ctx = rhs;
				ctx.r = *this ^ ctx.r;
				ctx.n = expand(ctx.n, 0.f) | transform;
				return ctx;
			}
		}

		template<typename T>
		requires std::is_same_v<std::remove_cvref_t<T>, Transform>
		auto operator^(T&& rhs) const -> Chain {
			return Chain{*this} ^ rhs;
		}
	};
}
