#include <metatron/render/photo/camera.hpp>

namespace mtt::photo {
    auto Camera::sample(
        Lens lens,
        cref<fv2> pos,
        cref<fv2> dxdy,
        cref<fv2> u
    ) noexcept -> Interaction {
        auto intr = Interaction{};

        {
            auto& ray = intr.ray_differential;
            auto r_pos = pos;
            auto rx_pos = r_pos + fv2{dxdy[0], 0.f};
            auto ry_pos = r_pos + fv2{0.f, dxdy[1]};

            auto r_intr = lens.sample(r_pos, u);
            auto rx_intr = lens.sample(rx_pos, u);
            auto ry_intr = lens.sample(ry_pos, u);

            ray.r = r_intr.r;
            ray.rx = rx_intr.r;
            ray.ry = ry_intr.r;
            intr.pdf = r_intr.pdf;
        }

        return intr;
    }
}
