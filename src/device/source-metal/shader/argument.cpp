#include "argument.hpp"
#include "../opaque/buffer.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        auto size = 0u;
        auto id = sizeof(MTL::ResourceID);
        for (auto i = 0uz; i < reflection.size(); ++i) {
            auto& refl = reflection[i];
            table[refl.path] = i;
            if (refl.type == Type::parameter) size += id;
            else size += math::clamp(refl.size, 1u, 8192u) * id;
        }
        set = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = command::Type::render,
            .size = size,
        });

        for (auto i = 0uz; i < reflection.size(); ++i) {
            auto& refl = reflection[i];
            if (refl.type == Type::parameter) {
                parameters = make_desc<opaque::Buffer>({
                    .state = opaque::Buffer::State::twin,
                    .type = command::Type::render,
                    .size = refl.size,
                });
                mut<uptr>(set->ptr)[i] = parameters->impl->device_buffer->gpuAddress();
                set->dirty.push_back({i * id, id});
            }
        }
    }

    auto Argument::index(std::string_view field) noexcept -> u32 {
        auto idx = table.find(field);
        if (idx == table.end()) stl::abort("field {} does not exits", field);
        return idx->second;
    }
}
