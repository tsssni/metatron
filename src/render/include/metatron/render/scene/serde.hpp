#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/descriptor.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/vector.hpp>
#include <metatron/core/stl/array.hpp>

namespace mtt::scene {
    template<typename T>
    auto deserialize(json&& j) {
        auto d = T{descriptor<T>{}};
        if (auto er = glz::read_json<T>(d, j.serialized.str); er) {
            std::println(
                "desrialize {} with glaze error: {}",
                j.serialized.str, glz::format_error(er)
            );
            std::abort();
        } else {
            Hierarchy::instance().attach(j.entity, std::move(d));
        }
    }

    template<typename... Ts>
    auto deserialize(
        std::array<std::string_view, sizeof...(Ts)>&& type,
        Hierarchy::binmap const& bins
    ) noexcept -> void {
        auto list = type
        | std::views::transform([&bins](auto x){return bins[x];})
        | std::views::join
        | std::ranges::to<std::vector<json>>();
        auto grid = math::Vector<usize, 1>{list.size()};
        stl::scheduler::instance().sync_parallel(grid, [&list, &type](auto idx) {
            auto [i] = idx;
            auto json = std::move(list[i]);
            [i, &json]<usize... idxs>(std::index_sequence<idxs...>) {
                ((i == idxs ? (deserialize<Ts>(std::move(json)), true) : false), ...);
            }(std::make_index_sequence<sizeof...(Ts)>{});
        });
    }

    template<
        typename T,
        typename... Ts,
        bool polied = (sizeof...(Ts) > 0) && (poliable<T, Ts> && ...)
    >
    auto deserialize(
        std::array<std::string_view, sizeof...(Ts) + usize(polied)>&& type,
        std::function<auto () -> void> pre = []{},
        std::function<auto () -> void> post = []{}
    ) noexcept -> void {
        Hierarchy::instance().filter([type = std::move(type), pre, post](auto bins) {
            using ts = std::conditional_t<polied, stl::array<Ts...>, stl::array<T, Ts...>>;
            if constexpr (sizeof...(Ts) > 0 && (poliable<T, Ts> && ...)) {
                auto& vec = stl::vector<T>::instance();
                (vec.template reserve<Ts>(bins[type[ts::template index<Ts>]].size()), ...);
            } else {
                stl::vector<T>::instance.reserve(bins[ts::template index<T>].size());
                (stl::vector<Ts>::instance().reserve(bins[type[ts::template index<Ts>]].size()), ...);
            }
            pre();
            deserialize(std::move(type), bins);
            post();
        });
    }
}
