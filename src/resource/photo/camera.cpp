#include <metatron/resource/photo/camera.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::photo {
    auto Camera::sample(
        math::Vector<usize, 2> pixel,
        usize idx,
        mut<math::Sampler> sampler
    ) noexcept -> std::optional<Interaction> {
        sampler->start(pixel, idx, 0);
        auto intr = Interaction{
            {}, {},
            (*film.data())(pixel, sampler->generate_pixel_2d()),
        };

        {
            auto& ray = intr.ray_differential;
            auto r_pos = math::Vector<f32, 2>{intr.fixel.position};
            auto rx_pos = r_pos + math::Vector<f32, 3>{intr.fixel.dxdy[0], 0.f};
            auto ry_pos = r_pos + math::Vector<f32, 3>{0.f, intr.fixel.dxdy[1]};

            MTT_OPT_OR_RETURN(r_intr, lens->sample(r_pos, sampler->generate_2d()), {});
            MTT_OPT_OR_RETURN(rx_intr, lens->sample(rx_pos, sampler->generate_2d()), {});
            MTT_OPT_OR_RETURN(ry_intr, lens->sample(ry_pos, sampler->generate_2d()), {});

            ray.differentiable = true;
            ray.r = r_intr.r;
            ray.rx = rx_intr.r;
            ray.ry = ry_intr.r;
        }

        {
            auto& ray = intr.default_differential;
            auto r_pos = math::Vector<f32, 2>{0.f};
            auto rx_pos = r_pos + math::Vector<f32, 2>{this->film->dxdy[0], 0.f};
            auto ry_pos = r_pos + math::Vector<f32, 2>{0.f, this->film->dxdy[1]};

            MTT_OPT_OR_RETURN(r_intr, lens->sample(r_pos, {0.f}), {});
            MTT_OPT_OR_RETURN(rx_intr, lens->sample(rx_pos, {0.f}), {});
            MTT_OPT_OR_RETURN(ry_intr, lens->sample(ry_pos, {0.f}), {});

            ray.differentiable = false;
            ray.r = r_intr.r;
            ray.rx = rx_intr.r;
            ray.ry = ry_intr.r;
        }

        return intr;
    }
}
