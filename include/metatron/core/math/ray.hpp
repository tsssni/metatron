#pragma once

namespace mtt::core {

struct Ray final {
    fv3 o;
    fv3 d;
};

struct Ray_Differential final {
    Ray r;
    Ray rx;
    Ray ry;
};

struct Ray_Context final {
    Ray r{};
    fv3 n{};
    fv4 lambda{};
    bool inside;
};

}
