#include "argument.hpp"
#include "../opaque/buffer.hpp"

namespace mtt::shader {
    Argument::Argument(cref<Descriptor> desc) noexcept {
        using Type = shader::Descriptor::Type;
        auto path = (stl::path{"shader"} / desc.name).concat(".json");
        stl::json::load(stl::filesystem::find(path), reflection);
        impl->offsets.resize(reflection.size());

        auto size = (u32)sizeof(MTL::ResourceID);
        auto slots = 0u;
        for (auto i = 0u; i < reflection.size(); ++i) {
            auto& refl = reflection[i];
            impl->offsets[i] = slots * size;
            slots += refl.type == Type::parameter
            ? 1u : (refl.count == 0 ? 8192u - slots : refl.count);
        }

        set = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = command::Type::render,
            .size = slots * size,
        });
        if (reflection.empty() || reflection.front().type != Type::parameter) return;

        impl->parameters = make_desc<opaque::Buffer>({
            .state = opaque::Buffer::State::twin,
            .type = command::Type::render,
            .size = reflection.front().size,
        });
        *mut<uptr>(set->ptr) = impl->parameters->impl->device_buffer->gpuAddress();
        set->dirty.push_back({0, size});
    }
}
