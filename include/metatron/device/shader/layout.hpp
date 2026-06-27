#pragma once
#include <metatron/core/stl/json.hpp>

namespace mtt::shader {
    struct Descriptor final {
        enum struct Type {
            parameter,
            sampler,
            image,
            grid,
            accel,
        } type;
        enum struct Access {
            readonly,
            readwrite,
        } access = Access::readonly;
        u32 size = 0;
        u32 offset = 0;
        u32 count = 1;
    };
    using Set = std::vector<Descriptor>;
}

namespace glz {
    template<>
    struct meta<mtt::shader::Descriptor::Type> {
        using enum mtt::shader::Descriptor::Type;
        auto constexpr static value = glz::enumerate(
            parameter, sampler, image, grid, accel
        );
    };

    template<>
    struct meta<mtt::shader::Descriptor::Access> {
        using enum mtt::shader::Descriptor::Access;
        auto constexpr static value = glz::enumerate(
            readonly, readwrite
        );
    };
}
