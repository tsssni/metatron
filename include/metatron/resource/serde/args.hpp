#pragma once
#include <string>

namespace mtt::scene {
    struct Args final {
        std::string scene;
        std::string output;
        std::string address;
        std::string device;
    };
}
