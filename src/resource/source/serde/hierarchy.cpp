#include <metatron/resource/serde/reflection.hpp>

namespace mtt::scene {
    struct Hierarchy::Impl final {
        std::vector<Hierarchy::filter_function> filters;
    };

    Hierarchy::Hierarchy() noexcept {}

    auto Hierarchy::filter(filter_function f) noexcept -> void {
        instance().impl->filters.push_back(std::move(f));
    }

    auto Hierarchy::populate(cref<stl::path> path) noexcept -> void {
        stl::filesystem::push(path);
        auto jsons = std::vector<scene::json>{};
        stl::json::load(path / "scene.json", jsons);
        auto bins = binmap{};
        for (auto& j: jsons) bins[j.type].push_back(std::move(j));
        for (auto& f: instance().impl->filters) f(bins);
    }
}
