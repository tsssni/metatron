#include <metatron/resource/serde/hierarchy.hpp>
#include <metatron/resource/serde/args.hpp>
#include <metatron/resource/serde/serde.hpp>
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/photo/film.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/stl/print.hpp>

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
        auto& vec = stl::vector<Local_Transform, Look_At_Transform>::instance();
        auto lces = vec.template keys<Local_Transform>();
        auto laes = vec.template keys<Look_At_Transform>();
        auto es = std::array{lces, laes} | std::views::join
        | std::views::transform([](auto&& x) { return std::string_view{x}; })
        | std::ranges::to<std::vector<std::string_view>>();

        stl::scheduler::instance().sync_parallel(uzv1{es.size()}, [&](auto idx) {
            auto [i] = idx;
            auto e = es[i];
            auto t = i < lces.size()
            ? math::Transform{*vec.template get<Local_Transform>(vec.template entity<Local_Transform>(e))}
            : math::Transform{*vec.template get<Look_At_Transform>(vec.template entity<Look_At_Transform>(e))};
            stl::vector<math::Transform>::instance().push(e, std::move(t));
        });
    }

    auto trace(
        math::proxy::Transform et,
        cref<std::unordered_map<u32, u32>> parents,
        cref<std::unordered_map<u32, std::vector<u32>>> children
    ) noexcept -> void {
        if (!children.contains(et)) return;
        auto& tvec = stl::vector<math::Transform>::instance();

        for (auto child: children.at(et)) {
            auto& t = tvec[child];
            t = math::Transform{et, t};
            trace(math::proxy::Transform{child}, parents, children);
        }
    }

    auto trace() noexcept -> void {
        auto& tvec = stl::vector<math::Transform>::instance();
        auto wt = *math::proxy::Transform::entity("/hierarchy/camera");
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

        tvec.push("/hierarchy/camera/render", std::move(ct));
        tvec.push("/hierarchy/shape", std::move(rt));
        tvec.push("/hierarchy/medium", std::move(rt));
        tvec.push("/hierarchy/light", std::move(rt));
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

        trace(math::proxy::Transform::entity("/hierarchy/shape"), parents, children);
        trace(math::proxy::Transform::entity("/hierarchy/medium"), parents, children);
        trace(math::proxy::Transform::entity("/hierarchy/light"), parents, children);
    }

    auto run() noexcept -> void {
        using namespace photo;
        auto& hierarchy = Hierarchy::instance();
        auto& args = Args::instance();

        MTT_DESERIALIZE_CALLBACK([]{}, []{
            merge(); trace();
        }, Local_Transform, Look_At_Transform);
        spectra::Spectrum::init();
        color::proxy::Color_Space::init();
        shape::Shape::init();
        volume::Volume::init();
        media::Medium::init();
        texture::Vector_Texture::init();
        texture::Spectrum_Texture::init();
        material::Material::init();
        bsdf::Bsdf::init();
        light::Light::init();

        accel::Acceleration::init();
        monte_carlo::Integrator::init();
        emitter::Emitter::init();
        sampler::Sampler::init();
        filter::Filter::init();
        photo::Lens::init();
        photo::proxy::Film::init();

        auto renderer = obj<renderer::Renderer>();
        auto gpu = args.device == "gpu";
        hierarchy.filter([&renderer, &gpu](auto bins){
            auto key = std::string{"renderer"};
            if (!bins.contains(key))
                stl::abort("renderer must be defined");
            auto j = bins[key].front();

            monte_carlo::Integrator::push<monte_carlo::Radiative_Integrator>("/renderer/default/integrator", {{}});
            accel::Acceleration::push<accel::HWBVH>("/renderer/default/accel", {{}});
            emitter::Emitter::push<emitter::Uniform_Emitter>("/renderer/default/emitter", {{}});
            sampler::Sampler::push<sampler::Z_Sobol_Sampler>("/renderer/default/sampler", {{}});
            filter::Filter::push<filter::Lanczos_Filter>("/renderer/default/filter", {{}});
            photo::Lens::push<photo::Thin_Lens>("/renderer/default/lens", {{}});

            using Descriptor = renderer::Renderer::Descriptor;
            auto desc = Descriptor{};
            stl::json::load(j.serialized.str, desc);

            if (gpu && !desc.accel.is<accel::HWBVH>()) {
                stl::print("fallback to HWBVH on gpu");
                desc.accel = accel::Acceleration::entity("/renderer/default/accel");
            } else if (!gpu && desc.accel.is<accel::HWBVH>()) {
                stl::print("fallback to LBVH on cpu");
                accel::Acceleration::push<accel::LBVH>("/renderer/default/bccel", {{}});
                desc.accel = accel::Acceleration::entity("/renderer/default/bccel");
            }
            renderer = make_obj<renderer::Renderer>(std::move(desc));
        });
        hierarchy.populate(args.scene);
        renderer->render();
    }
}
