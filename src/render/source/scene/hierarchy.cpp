#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/reflection.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <unordered_map>

namespace mtt::scene {
    struct Hierarchy::Impl final {
        std::vector<Hierarchy::filter_function> filters;
    };

    Hierarchy::Hierarchy() noexcept {}

    auto Hierarchy::filter(filter_function f) noexcept -> void {
        impl->filters.push_back(f);
    }

    auto Hierarchy::populate(cref<stl::path> path) noexcept -> void {
        stl::filesystem::push(path);
        auto jsons = std::vector<scene::json>{};
        stl::json::load(path / "scene.json", jsons);
        auto bins = binmap{};
        for (auto& j: jsons) bins[j.type].push_back(std::move(j));
        for (auto& f: impl->filters) f(bins);
    }
}
