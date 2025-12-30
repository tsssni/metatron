#include <metatron/render/scene/scene.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/args.hpp>
#include <metatron/render/scene/serde.hpp>
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/accel/hwbvh.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/sampler/independent.hpp>
#include <metatron/render/sampler/halton.hpp>
#include <metatron/render/sampler/sobol.hpp>
#include <metatron/render/filter/box.hpp>
#include <metatron/render/filter/gaussian.hpp>
#include <metatron/render/filter/lanczos.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto transform_init() noexcept -> void;
    auto spectra_init() noexcept -> void;
    auto shape_init() noexcept -> void;
    auto media_init() noexcept -> void;
    auto material_init() noexcept -> void;
    auto light_init() noexcept -> void;

    auto run() noexcept -> void {
        using namespace renderer;
        using namespace monte_carlo;
        using namespace accel;
        using namespace emitter;
        using namespace sampler;
        using namespace filter;
        using namespace photo;
        auto& hierarchy = Hierarchy::instance();
        auto& args = Args::instance();
        sampler::Sobol_Sampler::init();

        transform_init();
        spectra_init();
        shape_init();
        media_init();
        material_init();
        light_init();

        MTT_DESERIALIZE(Integrator, Volume_Path_Integrator);
        MTT_DESERIALIZE(Acceleration, LBVH, HWBVH);
        MTT_DESERIALIZE(Emitter, Uniform_Emitter);
        MTT_DESERIALIZE(Sampler, Independent_Sampler, Halton_Sampler, Sobol_Sampler);
        MTT_DESERIALIZE(Filter, Box_Filter, Gaussian_Filter, Lanczos_Filter);
        MTT_DESERIALIZE(Lens, Pinhole_Lens, Thin_Lens);
        MTT_DESERIALIZE(Film);

        auto renderer = obj<Renderer>();
        auto gpu = args.device == "gpu";
        hierarchy.filter([&renderer, &gpu](auto bins){
            auto key = std::string{"renderer"};
            if (!bins.contains(key))
                stl::abort("renderer must be defined");
            auto j = bins[key].front();

            stl::vector<Integrator>::instance().push<Volume_Path_Integrator>("/renderer/default/integrator", {{}});
            stl::vector<Acceleration>::instance().push<HWBVH>("/renderer/default/accel", {{}});
            stl::vector<Emitter>::instance().push<Uniform_Emitter>("/renderer/default/emitter", {{}});
            stl::vector<Sampler>::instance().push<Sobol_Sampler>("/renderer/default/sampler", {{}});
            stl::vector<Filter>::instance().push<Lanczos_Filter>("/renderer/default/filter", {{}});
            stl::vector<Lens>::instance().push<Thin_Lens>("/renderer/default/lens", {{}});

            using Descriptor = Renderer::Descriptor;
            auto desc = Descriptor{};
            stl::json::load(j.serialized.str, desc);

            if (gpu && !desc.accel.is<accel::HWBVH>()) {
                stl::print("fallback to HWBVH on gpu");
                desc.accel = entity<accel::Acceleration>("/renderer/default/accel");
            } else if (!gpu && desc.accel.is<accel::HWBVH>()) {
                stl::print("fallback to LBVH on cpu");
                stl::vector<accel::Acceleration>::instance().push<accel::LBVH>("/renderer/default/bccel", {{}});
                desc.accel = entity<accel::Acceleration>("/renderer/default/bccel");
            }
            renderer = make_obj<Renderer>(std::move(desc));
        });
        hierarchy.populate(args.scene);
        renderer->render();
    }
}
