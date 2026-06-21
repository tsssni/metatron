#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/function.hpp>
#include <metatron/core/math/vector.hpp>
#include <thread>
#include <atomic>
#include <future>
#include <mutex>
#include <stack>

namespace mtt::stl {
    struct scheduler final: singleton<scheduler> {
        scheduler(usize num_threads = std::thread::hardware_concurrency() - 1uz) noexcept {
            auto storage = std::vector<task>{};
            storage.reserve(64);
            threads.reserve(num_threads);
            tasks = decltype(tasks){std::move(storage)};
            for (auto i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] { while (true) {
                auto lock = std::unique_lock{mutex};
                cv.wait(lock, [this] { return !tasks.empty() || stop; });
                if (stop && tasks.empty()) break;
                auto task = std::move(tasks.top());
                tasks.pop(); lock.unlock(); (*task)();
            }});}
        }

        ~scheduler() noexcept {
            {
                auto lock = std::lock_guard{mutex};
                stop = true;
            }
            cv.notify_all();
            for (auto& t : threads) t.join();
        }

        template<typename F, usize size>
        requires (std::invocable<F, uzv<size>> && size >= 1 && size <= 3)
        auto static sync_parallel(cref<uzv<size>> grid, F&& f) noexcept {
            instance().parallel(grid, std::forward<F>(f), true).wait();
        }

        template<typename F, usize size>
        requires (std::invocable<F, uzv<size>> && size >= 1 && size <= 3)
        auto static async_parallel(cref<uzv<size>> grid, F&& f) noexcept {
            return instance().parallel(grid, std::forward<F>(f), false);
        }

        template<typename F>
        requires (std::invocable<F>)
        auto static async_dispatch(F&& f) noexcept {
            return instance().dispatch(std::forward<F>(f));
        }

        auto static index() noexcept -> usize {
            auto static thread_local tid = math::maxv<u32>;
            auto static aid = std::atomic<u32>{0};
            if (tid == math::maxv<u32>) tid = aid.fetch_add(1);
            return tid;
        }

        auto static size() noexcept -> usize { return instance().threads.size() + 1; }

    private:
        template<typename F, usize size>
        requires (true
        && std::invocable<F, uzv<size>>
        && std::same_as<std::invoke_result_t<F, uzv<size>>, void>
        && size >= 1 && size <= 3)
        auto parallel(cref<uzv<size>> grid, F&& f, bool sync) noexcept {
            using S = std::tuple<std::atomic<u32>, std::atomic<u32>, std::promise<void>>;
            auto state = std::make_unique<S>(0, 0, std::promise<void>{});
            auto& promise = std::get<2>(*state);
            auto future = promise.get_future();
            auto n = math::prod(grid);
            if (n == 0) { promise.set_value(); return future; }

            auto task = std::make_shared<function<void() noexcept>>([
                f = std::forward<F>(f),
                state = std::move(state),
                grid,
                n
            ]() mutable noexcept {
                auto& [index, dispatched, promise] = *state;
                auto i = 0u;
                auto finished = 0u;
                while ((i = index.fetch_add(1, std::memory_order::relaxed)) < n) {
                    if constexpr (size == 1) f(uzv1{i});
                    else if constexpr (size == 2) f(uzv2{i / grid[1], i % grid[1]});
                    else if constexpr (size == 3) f(uzv3{i / (grid[2] * grid[1]), (i / grid[2]) % grid[1], i % grid[2]});
                    ++finished;
                }
                auto total = dispatched.fetch_add(finished, std::memory_order::acq_rel) + finished;
                if (finished > 0 && total == n) promise.set_value();
            });

            {
                auto count = math::min(threads.size(), n - usize(sync));
                auto lock = std::lock_guard{mutex};
                for (auto i = 0uz; i < count; ++i) tasks.emplace(task);
            }

            cv.notify_all();
            if (sync) (*task)();
            return future;
        }

        template<typename F>
        requires (std::invocable<F>)
        auto dispatch(F&& f) noexcept {
            using R = std::invoke_result_t<F>;
            auto promise = std::make_unique<std::promise<R>>();
            auto future = promise->get_future().share();

            auto task = std::make_shared<function<void() noexcept>>([
                promise = std::move(promise),
                f = std::forward<F>(f)
            ]() mutable noexcept {
                if constexpr (!std::same_as<R, void>) promise->set_value(f());
                else { f(); promise->set_value(); }
            });

            {
                auto lock = std::lock_guard{mutex};
                tasks.emplace(task);
            }

            cv.notify_one();
            return future;
        }

        using task = std::shared_ptr<function<void() noexcept>>;
        std::mutex mutex;
        std::condition_variable cv;
        std::vector<std::thread> threads;
        std::stack<task, std::vector<task>> tasks;
        bool stop{false};
    };
}
