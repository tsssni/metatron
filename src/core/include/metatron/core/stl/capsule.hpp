#pragma once
#include <utility>

namespace mtt::stl {
	template<typename T>
	struct capsule {
		struct Impl final {
			template<typename... Args>
			Impl(Args&&... args): impl(new typename T::Impl(std::forward<Args>(args)...)) {}
			~Impl() {
				if (impl) {
					delete this->operator->();
				}
			}

			Impl(Impl&& impl) {
				*this = std::move(impl);
			}

			auto operator=(Impl&& impl) {
				if (impl) {
					delete this->operator->();
				}
				impl = impl.impl;
				impl.impl = nullptr;
				return *this;
			}

			auto operator->() {
				return (typename T::Impl*)impl;
			}

			auto operator->() const {
				return (typename T::Impl const*)impl;
			}

			auto operator*() {
				return *this->operator->();
			}

			auto operator*() const {
				return *this->operator->();
			}

		private:
			void* impl;
		};

	protected:
		Impl impl;
	};
}
