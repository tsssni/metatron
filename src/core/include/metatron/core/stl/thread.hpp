#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/math/vector.hpp>
#include <thread>
#include <atomic>
#include <future>
#include <mutex>
#include <functional>
#include <stack>

namespace mtt::stl {
	struct dispatcher final: singleton<dispatcher> {
		dispatcher(usize num_threads = std::thread::hardware_concurrency() - 1uz) noexcept {
			threads.reserve(num_threads);
			for (auto i = 0; i < num_threads; ++i) {
				threads.emplace_back([this]() {
					while (true) {
						auto lock = std::unique_lock{mutex};
						cv.wait(lock, [this] { return !tasks.empty() || stop; });
						if (stop && tasks.empty()) {
							break;
						}

						auto task = std::move(tasks.top());
						tasks.pop();
						lock.unlock();
						(*task)();
					}
				});
			}
		}

		~dispatcher() noexcept {
			{
				auto lock = std::lock_guard{mutex};
				stop = true;
			}
			cv.notify_all();
			for (auto& thread : threads) {
				thread.join();
			}
		}

		template<typename F, usize size>
		requires (std::is_invocable_v<F, math::Vector<usize, size>> && size >= 1 && size <= 3)
		auto sync_parallel(
			math::Vector<usize, size> const& grid,
			F&& f
		) noexcept -> void {
			auto future = parallel(grid, std::forward<F>(f), true);
			future.wait();
		}

		template<typename F, usize size>
		requires (std::is_invocable_v<F, math::Vector<usize, size>> && size >= 1 && size <= 3)
		auto async_parallel(
			math::Vector<usize, size> const& grid,
			F&& f
		) noexcept -> std::shared_future<void> {
			return parallel(grid, std::forward<F>(f), false);
		}

		template<
			typename F,
			usize size
		>
		requires (std::is_invocable_v<F, math::Vector<usize, size>> && size >= 1 && size <= 3)
		auto async_dispatch(
			math::Vector<usize, size> const& grid,
			F&& f
		) noexcept {
			using R = std::invoke_result_t<F, math::Vector<usize, size>>;
			auto promise = std::make_shared<std::promise<R>>();
			auto future = promise->get_future().share();
			
			auto task = std::make_shared<std::function<void()>>([
				promise = std::move(promise),
				future,
				f = std::forward<F>(f)
			]() mutable {
				if constexpr (std::same_as<R, void>) {
					f();
					promise->set_value();
				} else {
					promise->set_value(f());
				}
			});

			{
				auto lock = std::lock_guard{mutex};
				tasks.emplace(task);
			}

			cv.notify_one();
			return future;
		}

	private:
		template<typename F, usize size>
		requires (std::is_invocable_v<F, math::Vector<usize, size>> && size >= 1 && size <= 3)
		auto parallel(
			math::Vector<usize, size> const& grid,
			F&& f,
			bool sync
		) noexcept -> std::shared_future<void> {
			auto promise = std::make_shared<std::promise<void>>();
			auto future = promise->get_future().share();
			auto n = math::prod(grid);
			if (n == 0uz) {
				promise->set_value();
				return future;
			}

			auto task = std::make_shared<std::function<void()>>([
				f = std::forward<F>(f),
				index_counter = std::make_shared<std::atomic<u32>>(0u),
				dispatch_counter = std::make_shared<std::atomic<u32>>(0u),
				promise = std::move(promise),
				grid,
				n
			]() mutable {
				auto i = 0u;
				auto dispatched = 0u;
				while((i = index_counter->fetch_add(1)) < n) {
					if constexpr (size == 1) {
						f(math::Vector<usize, size>{i});
					} else if constexpr (size == 2) {
						f(math::Vector<usize, size>{
							i / grid[1],
							i % grid[1],
						});
					} else if constexpr (size == 3) {
						f(math::Vector<usize, size>{
							i / (grid[2] * grid[1]),
							(i / grid[2]) % grid[1],
							i % grid[2],
						});
					}
					dispatched++;
				}

				if (dispatched > 0u && dispatch_counter->fetch_add(dispatched) + dispatched == n) {
					promise->set_value();
				}
			});

			{
				auto lock = std::lock_guard{mutex};
				for (auto i = 0uz; i < std::min(threads.size(), n - usize(sync)); i++) {
					tasks.emplace(task);
				}
			}

			cv.notify_all();
			if (sync) {
				(*task)();
			}
			return future;
		}

		std::mutex mutex;
		std::condition_variable cv;
		std::vector<std::thread> threads;
		std::stack<std::shared_ptr<std::function<void()>>> tasks;
		bool stop{false};
	};
}
