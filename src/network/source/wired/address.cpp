#include <metatron/network/wired/address.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::wired {
    Address::Address(std::string_view address) noexcept {
        if (address.empty()) return;

        auto split = address.find_last_of(':');
        if (split == std::string::npos) {
            stl::print("wired address {} not in format host:port", address);
            return;
        }

        host = address.substr(0, split);
        port = address.substr(split + 1);
    }
}
