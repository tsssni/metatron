#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/photo/lens.hpp>
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::render {

struct Camera final {
    auto sample(
        Lens lens,
        fv2 const& pos,
        fv2 const& dxdy,
        fv2 const& u
    ) noexcept -> Camera_Interaction;
};

}
