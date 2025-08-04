#pragma once
#include <functional>

namespace mtt::stl {
	template<typename T>
	struct capsule {
		struct Impl final {
			template<typename... Args>
			Impl(Args&&... args) noexcept :
			impl(new typename T::Impl(std::forward<Args>(args)...)),
			deleter([](void* impl) {
				delete (typename T::Impl*)impl;
			}) {}

			~Impl() noexcept {
				if (impl) {
					deleter(impl);
				}
			}

			Impl(Impl&& impl) noexcept {
				*this = std::move(impl);
			}

			auto operator=(Impl&& impl) noexcept {
				if (impl) {
					deleter(impl);
				}
				impl = impl.impl;
				impl.impl = nullptr;
				return *this;
			}

			auto operator->() noexcept {
				return (typename T::Impl*)impl;
			}

			auto operator->() const noexcept {
				return (typename T::Impl const*)impl;
			}

			auto operator*() noexcept {
				return *this->operator->();
			}

			auto operator*() const noexcept {
				return *this->operator->();
			}

		private:
			void* impl;
			std::function<void(void*)> deleter;
		};

	protected:
		Impl impl;
	};
}
