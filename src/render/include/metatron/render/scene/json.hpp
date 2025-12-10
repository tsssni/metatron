#pragma once
#include <metatron/core/stl/json.hpp>

namespace mtt::scene {
    struct json final {
        std::string entity;
        std::string type;
        glz::raw_json serialized;
    };
}


