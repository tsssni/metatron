#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::scene {
    auto color_init() noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto& tvec = stl::vector<color::Transfer_Function>::instance();
        auto& cvec = stl::vector<color::Color_Space>::instance();

        auto transfer = std::to_array({
            [](f32 x) -> f32 {
                if (x <= 0.0031308f) return 12.92f * x;
                else return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
            }
        });
        auto linearize = std::to_array({
            [](f32 x) -> f32 {
                if (x <= 0.04045f) return x / 12.92f;
                else return std::pow((x + 0.055f) / 1.055f, 2.4f);
            }
        });

        for (auto i = 0uz; i < transfer.size(); i++)
            tvec.emplace_back(transfer[i], linearize[i]);

        auto name = std::to_array<std::string>({
            "sRGB",
        });
        auto red_primitive = std::to_array<math::Vector<f32, 2>>({
            {0.64f, 0.33f},
        });
        auto green_primitive = std::to_array<math::Vector<f32, 2>>({
            {0.30f, 0.60f},
        });
        auto blue_primitive = std::to_array<math::Vector<f32, 2>>({
            {0.15f, 0.06f},
        });
        auto white_point = std::to_array<std::string>({
            "CIE-D65",
        });
        auto transfer_function = std::to_array<u32>({
            0,
        });

        auto mutex = std::mutex{};
        auto grid = math::Vector<usize, 1>{name.size()};
        stl::scheduler::instance().sync_parallel(grid, [&](auto idx) {
            auto i = idx[0];
            auto entity = ("/color-space/" + name[i]) / et;
            auto white = ("/spectrum/" + white_point[i]) / et;

            auto cs = color::Color_Space{
                name[i],
                red_primitive[i],
                green_primitive[i],
                blue_primitive[i],
                hierarchy.fetch<spectra::Spectrum>(white),
                stl::proxy<color::Transfer_Function>{transfer_function[i]},
            };

            {
                auto lock = cvec.lock();
                hierarchy.attach<color::Color_Space>(entity, std::move(cs));
                auto cs = hierarchy.fetch<color::Color_Space>(entity);
                color::Color_Space::color_spaces[name[i]] = cs;
            }
        });
    }
}
