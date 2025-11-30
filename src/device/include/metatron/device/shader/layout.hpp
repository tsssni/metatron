#pragma once
#include <glaze/glaze.hpp>
#include <vector>

namespace mtt::shader {
    struct Layout final {
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
        std::vector<std::vector<Descriptor>> sets;
    };
}

namespace glz {
    template<>
    struct meta<mtt::shader::Layout::Descriptor::Type> {
        using enum mtt::shader::Layout::Descriptor::Type;
        auto constexpr static value = glz::enumerate(
            parameter, sampler, image, grid, accel
        );
    };

    template<>
    struct meta<mtt::shader::Layout::Descriptor::Access> {
        using enum mtt::shader::Layout::Descriptor::Access;
        auto constexpr static value = glz::enumerate(
            readonly, readwrite, writeonly
        );
    };
}
