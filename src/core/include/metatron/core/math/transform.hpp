#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/ray.hpp>
#include <vector>

namespace mtt::math {
	template<typename T>
    concept Transformable = false
	|| std::is_same_v<std::remove_cvref_t<T>, Vector<f32, 4>>
	|| std::is_same_v<std::remove_cvref_t<T>, Vector<f32, 3>> // for normal
	|| std::is_same_v<std::remove_cvref_t<T>, Ray>
	|| std::is_same_v<std::remove_cvref_t<T>, Ray_Differential>;

	struct Transform final {
		struct Config final {
			Vector<f32, 3> translation{};
			Vector<f32, 3> scaling{1.f};
			Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

			auto operator<=>(Config const& rhs) const = default;
		} config;

		struct Chain final {
			auto operator|(Transform const& t) noexcept -> Chain& {
				store(t);
				ops.push_back(0);
				return *this;
			}

			auto operator^(Transform const& t) noexcept -> Chain& {
				store(t);
				ops.push_back(1);
				return *this;
			}

			template<Transformable T>
			auto operator|(T const& t) {
				ops.push_back(0);
				return dechain(t);
			}

			template<Transformable T>
			auto operator^(T const& t) {
				ops.push_back(1);
				return dechain(t);
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
			Chain(Transform const& t) {
				store(t);
			}

			auto store(Transform const& t) noexcept -> void {
				transforms.push_back(&t);
			}

			template<Transformable T, typename Type = std::remove_cvref_t<T>>
			auto dechain(T const& rhs) noexcept -> Type {
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

		auto update() const noexcept -> void {
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
			} else if constexpr (std::is_same_v<Type, Vector<f32, 3>>) {
				return math::normalize(expand(rhs, 0.f) | inv_transform);
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

		auto operator|(Transform const& rhs) const noexcept -> Chain {
			return std::move(Chain{*this} | rhs);
		}

		template<Transformable T, typename Type = std::remove_cvref_t<T>>
		auto operator^(T&& rhs) const {
			if constexpr (std::is_same_v<Type, Vector<f32, 4>>) {
				return inv_transform | rhs;
			} else if constexpr (std::is_same_v<Type, Vector<f32, 3>>) {
				return math::normalize(expand(rhs, 0.f) | transform);
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
}
