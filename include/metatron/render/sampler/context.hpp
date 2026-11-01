#pragma once

namespace mtt::render {

struct Sampler_Context final {
    uv4 data;
    uv2 pixel;
    uv2 size;
    u32 idx;
    u32 spp;
    u32 dim;
    u32 seed;
};

}
