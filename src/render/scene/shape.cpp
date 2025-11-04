#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::scene {
    auto shape_init() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto& vec = stl::vector<shape::Shape>::instance();
        vec.emplace_type<shape::Mesh>();
        vec.emplace_type<shape::Sphere>();
    }
}
