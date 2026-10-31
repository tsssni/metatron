#pragma once

namespace mtt::phase {
    struct Interaction final {
        fv4 f;
        fv3 wi;
        f32 pdf;
    };
}
