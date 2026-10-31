#pragma once

namespace mtt::filter {
    struct Interaction final {
        fv2 p;
        f32 weight;
        f32 pdf;
    };
}
