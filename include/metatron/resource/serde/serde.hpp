#include <metatron/resource/serde/hierarchy.hpp>
#include <metatron/resource/serde/reflection.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    template<typename V, typename T>
    auto attach(cref<json> j, std::string_view type) noexcept -> bool {
        if (j.type != type) return false;
        auto d = T{};
        stl::json::load(j.serialized.str, d);
        V::template push<T>(j.entity, std::move(d));
        return true;
    }

    template<typename... Ts>
    auto constexpr deserialize(
        std::array<std::string, sizeof...(Ts)>&& type,
        auto (*pre)() -> void,
        auto (*post)() -> void
    ) noexcept -> void {
        stl::vector<Ts...>::init();
        Hierarchy::filter([type = std::move(type), pre, post](auto bins) {
            using ts = stl::array<Ts...>;
            pre();

            auto list = type
            | std::views::transform([&bins](auto x){return bins[x];})
            | std::views::join
            | std::ranges::to<std::vector<json>>();

            stl::scheduler::sync_parallel(uzv1{list.size()}, [&list, &type](auto idx) {
                auto [i] = idx;
                auto j = std::move(list[i]);
                auto v = (attach<stl::vector<Ts...>, Ts>(j, type[ts::template index<Ts>]) || ...);
                if (!v) stl::abort("deserialize {} with invalid type {}", j.serialized.str, j.type);
            });
            post();
        });
    }

    template<typename... Ts>
    auto constexpr deserialize(
        std::string_view text,
        auto (*pre)() -> void = []{},
        auto (*post)() -> void = []{}
    ) noexcept -> void {
        auto i = 0;
        auto type = std::array<std::string, sizeof...(Ts)>{};
        for (auto t: text
        | std::views::split(',')
        | std::views::transform([](auto&& x) {
            auto y = std::string{x.begin(), x.end()};
            auto l = y.find_first_not_of(" \t\r\n");
            auto r = y.find_last_not_of(" \t\r\n");
            if (l == std::string::npos) stl::abort("serialized type invalid");
            auto s = y.substr(l, r - l + 1);
            auto colon = s.rfind("::");
            return colon == std::string::npos ? s : s.substr(colon + 2);
        })) {
            std::ranges::transform(t, t.begin(), ::tolower);
            type[i++] = t;
        }
        deserialize<Ts...>(std::move(type), pre, post);
    }

    #define MTT_DESERIALIZE(...) mtt::scene::deserialize<__VA_ARGS__>(#__VA_ARGS__)
    #define MTT_DESERIALIZE_CALLBACK(pre, post, ...) mtt::scene::deserialize<__VA_ARGS__>(#__VA_ARGS__, pre, post)
}
