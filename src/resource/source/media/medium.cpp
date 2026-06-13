#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace glz {
    template<>
    struct meta<mtt::media::Phase::Function> {
        using enum mtt::media::Phase::Function;
        auto constexpr static value = glz::enumerate(henyey_greenstein);
    };
}

namespace mtt::media {
    auto Medium::init() noexcept -> void {
        MTT_DESERIALIZE(Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium);
        Medium::push<Vaccum_Medium>("/medium/vaccum", {});
    }

    auto Phase::to_phase() const noexcept -> phase::Phase_Function {
        switch (function) {
        case Phase::Function::henyey_greenstein:
            return phase::Phase_Function{phase::Henyey_Greenstein_Phase_Function{g}};
        default: return {};
        };
    }
}
