#include "accel.hpp"
#include "transfer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/buffer.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Acceleration_Encoder::Acceleration_Encoder(
        mut<command::Buffer> cmd, mut<opaque::Acceleration> accel
    ) noexcept: cmd(cmd), accel(accel) { impl->encoder = cmd->impl->cmd->accelerationStructureCommandEncoder(); }
    auto Acceleration_Encoder::submit() noexcept -> void { impl->encoder->endEncoding(); }
    auto Acceleration_Encoder::persist() noexcept -> void {}

    auto Acceleration_Encoder::build() noexcept -> void {
        auto transfer = Transfer_Encoder{cmd};
        transfer.upload(*accel->instances);
        if (accel->bboxes) transfer.upload(*accel->bboxes);
        transfer.submit();

        for (auto i = 0; i < accel->scratches.size() - 1; ++i)
            impl->encoder->buildAccelerationStructure(
                accel->impl->primitives[i].get(),
                accel->impl->primitives_descs[i].get(),
                accel->scratches[i]->impl->device_buffer.get(), 0
            );

        impl->encoder->buildAccelerationStructure(
            accel->impl->instances.get(),
            accel->impl->instances_desc.get(),
            accel->scratches.back()->impl->device_buffer.get(), 0
        );
    }
}
