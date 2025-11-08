#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/descriptor.hpp>
#include <metatron/render/scene/reflection.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    template<typename... Ts>
    auto constexpr deserialize(
        std::array<std::string, sizeof...(Ts)>&& type,
        std::function<auto () -> void> pre,
        std::function<auto () -> void> post
    ) noexcept -> void {
        Hierarchy::instance().filter([type = std::move(type), pre, post](auto bins) {
            using ts = stl::array<Ts...>;
            (stl::vector<Ts>::instance().reserve(bins[type[ts::template index<Ts>]].size()), ...);
            pre();

            auto list = type
            | std::views::transform([&bins](auto x){return bins[x];})
            | std::views::join
            | std::ranges::to<std::vector<json>>();

            auto grid = math::Vector<usize, 1>{list.size()};
            stl::scheduler::instance().sync_parallel(grid, [&list, &type](auto idx) {
                auto [i] = idx;
                auto j = std::move(list[i]);
                auto v = ((j.type == type[ts::template index<Ts>] ? ([&j]{
                    auto d = Ts{descriptor_t<Ts>{}};
                    if (auto er = glz::read_json<Ts>(d, j.serialized.str); er) {
                        std::println(
                            "desrialize {} with glaze error: {}",
                            j.serialized.str, glz::format_error(er)
                        );
                        std::abort();
                    } else {
                        auto lock = stl::vector<Ts>::instance().lock();
                        Hierarchy::instance().attach<Ts, Ts>(j.entity, std::move(d));
                    }
                }(), true) : false) || ...);
                if (!v) std::println(
                    "desrialize {} with invalid type {}",
                    j.serialized.str, j.type
                );
            });
            post();
        });
    }

    template<pro::facade F, typename... Ts>
    auto constexpr deserialize(
        std::array<std::string, sizeof...(Ts) + 1>&& type,
        std::function<auto () -> void> pre,
        std::function<auto () -> void> post
    ) noexcept -> void {
        Hierarchy::instance().filter([type = std::move(type), pre, post](auto bins) {
            using ts = stl::array<F, Ts...>;
            (stl::vector<F>::instance().template reserve<Ts>(bins[type[ts::template index<Ts>]].size()), ...);
            pre();

            auto list = type
            | std::views::transform([&bins](auto x){return bins[x];})
            | std::views::join
            | std::ranges::to<std::vector<json>>();

            auto grid = math::Vector<usize, 1>{list.size()};
            stl::scheduler::instance().sync_parallel(grid, [&list, &type](auto idx) {
                auto [i] = idx;
                auto j = std::move(list[i]);
                auto v = ((j.type == type[ts::template index<Ts>] ? ([&j]{
                    auto d = Ts{descriptor_t<Ts>{}};
                    if (auto er = glz::read_json<Ts>(d, j.serialized.str); er) {
                        std::println(
                            "desrialize {} of type {} with glaze error: {}",
                            j.serialized.str, j.type, glz::format_error(er)
                        );
                        std::abort();
                    } else {
                        auto lock = stl::vector<F>::instance().template lock<Ts>();
                        Hierarchy::instance().attach<F, Ts>(j.entity, std::move(d));
                    }
                }(), true) : false) || ...);
                if (!v) std::println(
                    "desrialize {} with invalid type {}",
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
