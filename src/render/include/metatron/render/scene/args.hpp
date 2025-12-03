#pragma once
#include <metatron/core/stl/singleton.hpp>

namespace mtt::scene {
    struct Args final: stl::singleton<Args> {
        std::string scene;
        std::string output;
        std::string address;
        std::string device;
    };
}
