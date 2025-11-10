#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/descriptor.hpp>
#include <metatron/render/scene/reflection.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    template<typename F, typename T = F>
    auto reserve(usize n) noexcept -> void {
        if constexpr (pro::facade<F>) {
            auto& vec = stl::vector<F>::instance();
            vec.template reserve<T>(vec.template size<T>() + n);
        } else {
            auto& vec = stl::vector<T>::instance();
            vec.reserve(vec.size() + n);
        }
    }

    template<typename F, typename T = F>
    auto attach(json const& j, std::string_view type) noexcept -> bool {
        if (j.type != type) return false;
        auto d = T{};
        if (auto er = glz::read_json<T>(d, j.serialized.str); er) {
            std::println(
                "deserialize {} with glaze error: {}",
                j.serialized.str, glz::format_error(er)
            );
            std::abort();
        } else {
            if constexpr (pro::facade<F>) {
                auto lock = stl::vector<F>::instance().lock();
                attach<F, T>(j.entity, std::move(d));
            } else {
                auto lock = stl::vector<T>::instance().lock();
                attach<T>(j.entity, std::move(d));
            }
        }
        return true;
    }

    template<typename F, typename... Ts>
    auto constexpr deserialize(
        std::array<std::string, sizeof...(Ts) + 1>&& type,
        std::function<auto () -> void> pre,
        std::function<auto () -> void> post
    ) noexcept -> void {
        Hierarchy::instance().filter([type = std::move(type), pre, post](auto bins) {
            using ts = stl::array<F, Ts...>;
            if constexpr (!pro::facade<F>)
                reserve<F>(bins[type[ts::template index<F>]].size());
            (reserve<F, Ts>(bins[type[ts::template index<Ts>]].size()), ...);
            pre();

            auto list = type
            | std::views::transform([&bins](auto x){return bins[x];})
            | std::views::join
            | std::ranges::to<std::vector<json>>();

            auto grid = math::Vector<usize, 1>{list.size()};
            stl::scheduler::instance().sync_parallel(grid, [&list, &type](auto idx) {
                auto [i] = idx;
                auto j = std::move(list[i]);
                auto v = false;
                if constexpr (!pro::facade<F>)
                    v = attach<F>(j, type[ts::template index<F>]);
                v = v || (attach<F, Ts>(j, type[ts::template index<Ts>]) || ...);
                if (!v) std::println(
                    "deserialize {} with invalid type {}",
                    j.serialized.str, j.type
                );
            });
            post();
        });
    }

    template<typename... Ts>
    auto constexpr deserialize(
        std::string_view text,
        std::function<auto () -> void> pre = []{},
        std::function<auto () -> void> post = []{}
    ) noexcept -> void {
        auto i = 0;
        auto type = std::array<std::string, sizeof...(Ts)>{};
        for (auto t: text
        | std::views::split(',')
        | std::views::transform([](auto&& x) {
            auto y = std::string{x.begin(), x.end()};
            auto l = y.find_first_not_of(" \t\r\n");
            auto r = y.find_last_not_of(" \t\r\n");
            if (l == std::string::npos) {
                std::println("serialized type invalid");
                std::abort();
            } else return y.substr(l, r - l + 1);
        })) {
            std::ranges::transform(t, t.begin(), ::tolower);
            type[i++] = t;
        }
        deserialize<Ts...>(std::move(type), pre, post);
    }

    #define MTT_DESERIALIZE(...) deserialize<__VA_ARGS__>(#__VA_ARGS__)
    #define MTT_DESERIALIZE_CALLBACK(pre, post, ...) deserialize<__VA_ARGS__>(#__VA_ARGS__, pre, post)
}
