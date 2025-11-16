#include <metatron/render/photo/camera.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::photo {
    auto Camera::sample(
        view<Lens> lens,
        fv2 pos,
        fv2 dxdy,
        fv2 u
    ) noexcept -> opt<Interaction> {
        auto intr = Interaction{};

        {
            auto& ray = intr.ray_differential;
            auto r_pos = pos;
            auto rx_pos = pos + fv3{dxdy[0], 0.f};
            auto ry_pos = pos + fv3{0.f, dxdy[1]};

            MTT_OPT_OR_RETURN(r_intr, lens->sample(r_pos, u), {});
            MTT_OPT_OR_RETURN(rx_intr, lens->sample(rx_pos, u), {});
            MTT_OPT_OR_RETURN(ry_intr, lens->sample(ry_pos, u), {});

            ray.differentiable = true;
            ray.r = r_intr.r;
            ray.rx = rx_intr.r;
            ray.ry = ry_intr.r;
            intr.pdf = r_intr.pdf;
        }

        {
            auto& ray = intr.default_differential;
            auto r_pos = fv2{0.f};
            auto rx_pos = r_pos + fv2{dxdy[0], 0.f};
            auto ry_pos = r_pos + fv2{0.f, dxdy[1]};

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
