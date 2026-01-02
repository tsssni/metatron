#include "argument.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto offset = 0uz;
        impl->offsets.resize(reflection.size());
        for (auto i = 0uz; i < reflection.size(); ++i) {
            impl->offsets[i] = offset;
            auto size = 0uz;
            auto& refl = reflection[i];
            if (refl.type == Type::parameter) {
                parameters = make_desc<opaque::Buffer>({
                    .state = opaque::Buffer::State::twin,
                    .type = command::Type::render,
                    .size = refl.size,
                });
            };
            offset += sizeof(uptr);
        }

        set = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = command::Type::render,
            .size = offset,
        });
    }

    auto Argument::index(std::string_view field) noexcept -> u32 {
        auto idx = table.find(field);
        if (idx == table.end()) stl::abort("field {} does not exits", field);
        return idx->second;
    }
}
