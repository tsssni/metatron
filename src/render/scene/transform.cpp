#include <metatron/render/scene/serde.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::scene {
    struct Local_Transform final {
        math::Vector<f32, 3> translation{0.f};
        math::Vector<f32, 3> scaling{1.f};
        math::Quaternion<f32> rotation{0.f, 0.f, 0.f, 1.f};

        operator math::Transform() const noexcept {
            auto translation = math::Matrix<f32, 4, 4>{
                {1.f, 0.f, 0.f, this->translation[0]},
                {0.f, 1.f, 0.f, this->translation[1]},
                {0.f, 0.f, 1.f, this->translation[2]},
                {0.f, 0.f, 0.f, 1.f},
            };
            auto scaling = math::Matrix<f32, 4, 4>{
                this->scaling[0], this->scaling[1], this->scaling[2], 1.f
            };
            auto rotation = math::Matrix<f32, 4, 4>{this->rotation};
            return math::Transform{translation | rotation | scaling};
        }
    };

    struct Look_At_Transform final {
        math::Vector<f32, 3> position{0.f};
        math::Vector<f32, 3> look_at{0.f, 0.f, 1.f};
        math::Vector<f32, 3> up{0.f, 1.f, 0.f};

        operator math::Transform() const noexcept {
            auto forward = math::normalize(look_at - position);
            auto right = math::normalize(math::cross(up, forward));
            auto up = math::normalize(math::cross(forward, right));
            return math::Transform{math::Matrix<f32, 4, 4>{
                {right[0], up[0], forward[0], position[0]},
                {right[1], up[1], forward[1], position[1]},
                {right[2], up[2], forward[2], position[2]},
                {0.f,      0.f,   0.f,        1.f},
            }};
        }
    };

    auto merge() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();
        auto lces = hierarchy.entities<Local_Transform>();
        auto laes = hierarchy.entities<Look_At_Transform>();

        auto es = std::vector<Entity>{};
        std::ranges::copy(lces, std::back_inserter(es));
        std::ranges::copy(laes, std::back_inserter(es));

        auto size = lces.size() + laes.size();
        auto grid = math::Vector<usize, 1>{size};
        auto& tv = stl::vector<math::Transform>::instance();
        tv.reserve(size);

        stl::scheduler::instance().sync_parallel(grid, [&](auto idx) {
            auto [i] = idx;
            auto e = es[i];
            auto t = i < lces.size()
            ? math::Transform{*hierarchy.fetch<Local_Transform>(e).data()}
            : math::Transform{*hierarchy.fetch<Look_At_Transform>(e).data()};

            auto lock = tv.lock();
            hierarchy.attach<math::Transform>(e, std::move(t));
        });
    }

    auto trace(Entity et) noexcept -> void {
        auto& hierarchy = Hierarchy::instance();
        auto& rt = *hierarchy.fetch<math::Transform>(et).data();

        for (auto child: hierarchy.children(et)) {
            auto& t = *hierarchy.fetch<math::Transform>(child).data();
            t = math::Transform{rt, t};
            trace(child);
        }
    }

    auto trace() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto wt = *hierarchy.fetch<math::Transform>("/hierarchy/camera"_et).data();
        auto t = wt.transform;
        auto inv_t = wt.inv_transform;

        auto rt = math::Transform{Local_Transform{
            .translation = -math::Vector<f32, 3>{t[0][3], t[1][3], t[2][3]},
        }};
        t[0][3] = t[1][3] = t[2][3] = 0.f;
        inv_t[0][3] = inv_t[1][3] = inv_t[2][3] = 0.f;

        auto ct = math::Transform{};
        ct.inv_transform = t;
        ct.transform = inv_t;

        hierarchy.attach<math::Transform>("/hierarchy/camera/render"_et, ct);
        hierarchy.attach<math::Transform>("/hierarchy/shape"_et, rt);
        hierarchy.attach<math::Transform>("/hierarchy/medium"_et, rt);
        hierarchy.attach<math::Transform>("/hierarchy/light"_et, rt);

        trace("/hierarchy/shape"_et);
        trace("/hierarchy/medium"_et);
        trace("/hierarchy/light"_et);
    }

    auto transform_init() noexcept -> void {
        MTT_DESERIALIZE_CALLBACK([]{}, []{
            merge(); trace();
        }, Local_Transform, Look_At_Transform);
    }
}
