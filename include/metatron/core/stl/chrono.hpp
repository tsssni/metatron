#pragma once
#include <metatron/core/stl/print.hpp>
#include <atomic>
#include <chrono>
#include <cstdio>

namespace mtt::stl {
    using nanoseconds = std::ratio<1, 1000000000>;
    using microseconds = std::ratio<1, 1000000>;
    using milliseconds = std::ratio<1, 1000>;
    using seconds = std::ratio<1, 1>;
    using minutes = std::ratio<60, 1>;
    using hours = std::ratio<3600, 1>;
    using days = std::ratio<86400, 1>;

    struct timer final {
        template<typename T, typename... Rs>
        auto ts() noexcept {
            using namespace std::chrono;
            auto now = steady_clock::now();
            return std::make_tuple(duration_cast<duration<T, Rs>>(now - start).count()...);
        }

        template<typename T, typename R>
        auto t() noexcept { auto [t] = ts<T, R>(); return t; }

    private:
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    };

    struct progress final {
        progress(usize total) noexcept: total(total) {}

        auto operator+(usize size) noexcept -> void {
            auto count = atomic_count.fetch_add(size) + 1;
            auto percent = static_cast<usize>(100.f * count / total);

            auto last = atomic_percent.load();
            if (percent > last && atomic_percent.compare_exchange_weak(last, percent)) {
                auto [ms, s, mi] = timer.ts<usize, milliseconds, seconds, minutes>();
                auto total_s = ms * 100 / percent / 1000;
                std::print("\rprogress: {}% time: [{:02d}:{:02d}/{:02d}:{:02d}]",
                    percent, mi, s % 60, total_s / 60, total_s % 60
                );
                std::fflush(stdout);
            }
        }

        auto operator++() noexcept -> void { *this + 1; }
        auto operator~() noexcept -> void { std::println(); }

    private:
        std::atomic<usize> atomic_count{};
        std::atomic<usize> atomic_percent{};
        stl::timer timer; 
        usize total;
    };
}
