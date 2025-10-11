#include <metatron/core/stl/print.hpp>
#include <atomic>
#include <chrono>
#include <iostream>

namespace mtt::stl {
    struct progress final {
        progress(usize total) noexcept
        : total(total) {
            start_time = std::chrono::system_clock::now();
        }

        auto operator++() noexcept -> void {
            auto count = atomic_count.fetch_add(1) + 1;
            auto percent = static_cast<usize>(100.f * count / total);
            
            auto last_percent = atomic_percent.load();
            if (percent > last_percent && atomic_percent.compare_exchange_weak(last_percent, percent)) {
                auto time = std::chrono::system_clock::now();
                auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time - start_time).count();
                auto total_ms = elapsed_ms + (100 - percent) * elapsed_ms / percent;
                auto elapsed_s = elapsed_ms / 1000;
                auto total_s = total_ms / 1000;
                std::print("\rprogress: {}% time: [{:02d}:{:02d}/{:02d}:{:02d}]",
                    percent, elapsed_s / 60, elapsed_s % 60, total_s / 60, total_s % 60
                );
                std::flush(std::cout);
            }
        }

        auto operator~() noexcept -> void {
            std::println();
        }

    private:
        std::atomic<usize> atomic_count{0uz};
        std::atomic<usize> atomic_percent{0uz};
        usize total;
        std::chrono::system_clock::time_point start_time;
    };
}
