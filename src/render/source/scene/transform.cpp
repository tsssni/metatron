#include <metatron/render/scene/serde.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::scene {
    struct Local_Transform final {
        fv3 translation{0.f};
        fv3 scaling{1.f};
        fq rotation{0.f, 0.f, 0.f, 1.f};

        operator math::Transform() const noexcept {
            auto translation = fm44{
                {1.f, 0.f, 0.f, this->translation[0]},
                {0.f, 1.f, 0.f, this->translation[1]},
                {0.f, 0.f, 1.f, this->translation[2]},
                {0.f, 0.f, 0.f, 1.f},
            };
            auto scaling = fm44{
                this->scaling[0], this->scaling[1], this->scaling[2], 1.f
            };
            auto rotation = fm44{this->rotation};
            return math::Transform{translation | rotation | scaling};
        }
    };

    struct Look_At_Transform final {
        fv3 position{0.f};
        fv3 look_at{0.f, 0.f, 1.f};
        fv3 up{0.f, 1.f, 0.f};

        operator math::Transform() const noexcept {
            auto forward = math::normalize(look_at - position);
            auto right = math::normalize(math::cross(up, forward));
            auto up = math::normalize(math::cross(forward, right));
            return math::Transform{fm44{
                {right[0], up[0], forward[0], position[0]},
                {right[1], up[1], forward[1], position[1]},
                {right[2], up[2], forward[2], position[2]},
                {0.f,      0.f,   0.f,        1.f},
            }};
        }
    };

    auto merge() noexcept -> void {
        auto lces = stl::vector<Local_Transform>::instance().keys();
        auto laes = stl::vector<Look_At_Transform>::instance().keys();
        auto es = std::array{lces, laes} | std::views::join
        | std::views::transform([](auto&& x) { return std::string_view{x}; })
        | std::ranges::to<std::vector<std::string_view>>();

        stl::scheduler::instance().sync_parallel(uzv1{es.size()}, [&](auto idx) {
            auto [i] = idx;
            auto e = es[i];
            auto t = i < lces.size()
            ? math::Transform{*entity<Local_Transform>(e)}
            : math::Transform{*entity<Look_At_Transform>(e)};
            stl::vector<math::Transform>::instance().push(e, std::move(t));
        });
    }

    auto trace(
        tag<math::Transform> et,
        cref<std::unordered_map<u32, u32>> parents,
        cref<std::unordered_map<u32, std::vector<u32>>> children
    ) noexcept -> void {
        if (!children.contains(et)) return;
        auto& tvec = stl::vector<math::Transform>::instance();
        auto& rt = *et;

        for (auto child: children.at(et)) {
            auto& t = *tvec[child];
            t = math::Transform{rt, t};
            trace(child, parents, children);
        }
    }

    auto trace() noexcept -> void {
        auto& tvec = stl::vector<math::Transform>::instance();
        auto wt = *entity<math::Transform>("/hierarchy/camera");
        auto t = wt.transform;
        auto inv_t = wt.inv_transform;

        auto rt = math::Transform{Local_Transform{
            .translation = -fv3{t[0][3], t[1][3], t[2][3]},
        }};
        t[0][3] = t[1][3] = t[2][3] = 0.f;
        inv_t[0][3] = inv_t[1][3] = inv_t[2][3] = 0.f;

        auto ct = math::Transform{};
        ct.inv_transform = t;
        ct.transform = inv_t;

        tvec.push("/hierarchy/camera/render", ct);
        tvec.push("/hierarchy/shape", rt);
        tvec.push("/hierarchy/medium", rt);
        tvec.push("/hierarchy/light", rt);
        tvec.push("/hierarchy/medium/vaccum", {});

        auto parents = std::unordered_map<u32, u32>{};
        auto children = std::unordered_map<u32, std::vector<u32>>{};
        for (auto& path: tvec.keys()) {
            auto slash = path.find_last_of('/');
            if (slash == std::string::npos) stl::abort("ecs: invalid path {}", path);
            auto entity = tvec.entity(path);
            auto fath = slash == 0 ? "/" : path.substr(0, slash);
            if (!tvec.contains(fath)) continue;
            auto parent = tvec.entity(fath);
            parents[entity] = parent;
            children[parent].push_back(entity);
        }

        trace(tvec.entity("/hierarchy/shape"), parents, children);
        trace(tvec.entity("/hierarchy/medium"), parents, children);
        trace(tvec.entity("/hierarchy/light"), parents, children);
    }

    auto transform_init() noexcept -> void {
        MTT_DESERIALIZE_CALLBACK([]{}, []{
            merge(); trace();
        }, Local_Transform, Look_At_Transform);
    }
}
