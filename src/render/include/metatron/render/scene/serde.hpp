#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/reflection.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    template<typename F, typename T = F>
    auto attach(cref<json> j, std::string_view type) noexcept -> bool {
        if (j.type != type) return false;
        auto d = T{};
        stl::json::load(j.serialized.str, d);
        if constexpr (pro::facade<F>)
            stl::vector<F>::instance().template push<T>(j.entity, std::move(d));
        else
            stl::vector<T>::instance().push(j.entity, std::move(d));
        return true;
    }

    template<typename F, typename... Ts>
    auto constexpr deserialize(
        std::array<std::string, sizeof...(Ts) + 1>&& type,
        std::function<void()> pre,
        std::function<void()> post
    ) noexcept -> void {
        auto& vec = stl::vector<F>::instance();
        if constexpr (pro::facade<F>) (vec.template emplace_type<Ts>(), ...);
        else (stl::vector<Ts>::instance(), ...);
        Hierarchy::instance().filter([type = std::move(type), pre, post](auto bins) {
            using ts = stl::array<F, Ts...>;
            pre();

            auto list = type
            | std::views::transform([&bins](auto x){return bins[x];})
            | std::views::join
            | std::ranges::to<std::vector<json>>();

            stl::scheduler::instance().sync_parallel(uzv1{list.size()}, [&list, &type](auto idx) {
                auto [i] = idx;
                auto j = std::move(list[i]);
                auto v = false;
                if constexpr (!pro::facade<F>)
                    v = attach<F>(j, type[ts::template index<F>]);
                v = v || (attach<F, Ts>(j, type[ts::template index<Ts>]) || ...);
                if (!v) stl::abort(
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
        std::function<void()> pre = []{},
        std::function<void()> post = []{}
    ) noexcept -> void {
        auto i = 0;
        auto type = std::array<std::string, sizeof...(Ts)>{};
        for (auto t: text
        | std::views::split(',')
        | std::views::transform([](auto&& x) {
            auto y = std::string{x.begin(), x.end()};
            auto l = y.find_first_not_of(" \t\r\n");
            auto r = y.find_last_not_of(" \t\r\n");
            if (l == std::string::npos)
                stl::abort("serialized type invalid");
            return y.substr(l, r - l + 1);
        })) {
            std::ranges::transform(t, t.begin(), ::tolower);
            type[i++] = t;
        }
        deserialize<Ts...>(std::move(type), pre, post);
    }

    #define MTT_DESERIALIZE(...) deserialize<__VA_ARGS__>(#__VA_ARGS__)
    #define MTT_DESERIALIZE_CALLBACK(pre, post, ...) deserialize<__VA_ARGS__>(#__VA_ARGS__, pre, post)
}
