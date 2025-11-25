#include <metatron/device/shader/compiler.hpp>
#include <slang-com-ptr.h>
#include <slang.h>

namespace mtt::shader {
    struct Compiler::Impl final {
        Slang::ComPtr<slang::IGlobalSession> global_session;

        Impl() noexcept {
            slang::createGlobalSession(global_session.writeRef());
        }
    };

    Compiler::Compiler() noexcept {}
}
