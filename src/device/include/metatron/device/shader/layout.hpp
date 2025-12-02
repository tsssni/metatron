#pragma once
#include <metatron/core/stl/json.hpp>

namespace mtt::shader {
    struct Descriptor final {
        std::string path;
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
            writeonly,
        } access = Access::readonly;
        // parameter: struct size
        // array: size > 0 or -1 for bindless
        i32 size = 0;
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
            readonly, readwrite, writeonly
        );
    };
}
