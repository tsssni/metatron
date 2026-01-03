#include "argument.hpp"
#include "../opaque/buffer.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        using Type = shader::Descriptor::Type;
        using Access = shader::Descriptor::Access;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);

        set = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = command::Type::render,
            .size = reflection.size() * sizeof(MTL::ResourceID),
        });

        for (auto i = 0uz; i < reflection.size(); ++i) {
            auto size = 0uz;
            auto& refl = reflection[i];
            table[refl.path] = i;
            if (refl.type == Type::parameter) {
                parameters = make_desc<opaque::Buffer>({
                    .state = opaque::Buffer::State::twin,
                    .type = command::Type::render,
                    .size = refl.size,
                });
                mut<uptr>(set->ptr)[i] = parameters->impl->device_buffer->gpuAddress();
            } else if (refl.size == math::maxv<u32>) {
                bindless = make_desc<opaque::Buffer>({
                    .state = opaque::Buffer::State::twin,
                    .type = command::Type::render,
                    .size = sizeof(MTL::ResourceID) * 8192,
                });
                mut<uptr>(set->ptr)[i] = bindless->impl->device_buffer->gpuAddress();
            }
        }
    }

    auto Argument::index(std::string_view field) noexcept -> u32 {
        auto idx = table.find(field);
        if (idx == table.end()) stl::abort("field {} does not exits", field);
        return idx->second;
    }
}
