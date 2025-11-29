#pragma once
#include <glaze/glaze.hpp>
#include <vector>

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
        i32 set = -1;
        i32 index = -1;
    };

    struct Layout final {
        std::vector<Descriptor> descriptors;
    };
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
