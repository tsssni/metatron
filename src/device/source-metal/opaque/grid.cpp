#include "grid.hpp"

namespace mtt::opaque {
    Grid::Grid(cref<Descriptor> desc) noexcept {}

    Grid::operator View() noexcept {
        return {this, {0}, {width, height, depth}};
    }
}
