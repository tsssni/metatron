#include <metatron/device/shader/compiler.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <filesystem>
#include <fstream>
#include <cstring>

namespace mtt::shader {
    #ifdef __linux__
        auto constexpr format = SLANG_SPIRV;
        auto constexpr ext = std::string_view{".spirv"};
    #elif __APPLE__
        auto constexpr format = SLANG_METAL_LIB;
        auto constexpr ext = std::string_view{".metallib"};
    #endif
    auto constexpr kernel = std::string_view{".kernel.slang"};

    struct Compiler::Impl final {
        std::filesystem::path dir;
        std::filesystem::path out;
        Slang::ComPtr<slang::IGlobalSession> global_session;
        Slang::ComPtr<slang::ISession> session;

        Impl() noexcept {}

        auto compile(mut<slang::IModule> module, i32 idx, std::filesystem::path src) noexcept -> void {
            auto diagnostic = Slang::ComPtr<slang::IBlob>{};
            auto guard = [](SlangResult result) {
                if (SLANG_FAILED(result)) {
                    std::println("slang compilation failed with {}", result);
                    std::abort();
                }
            };
            auto print = [&diagnostic, &guard](SlangResult result) {
                if (diagnostic && diagnostic->getBufferSize() > 0)
                    std::println("{}", view<char>(diagnostic->getBufferPointer()));
                guard(result);
            };

            auto entry = Slang::ComPtr<slang::IEntryPoint>{};
            auto hash = Slang::ComPtr<slang::IBlob>{};
            guard(module->getDefinedEntryPoint(idx, entry.writeRef()));
            module->getEntryPointHash(idx, 0, hash.writeRef());
            if (!hash || hash->getBufferSize() == 0) {
                std::println(
                    "slang failed to generate hash for {} {}",
                    src.c_str(), entry->getFunctionReflection()->getName()
                );
                std::abort();
            }

            auto file = src.parent_path() / std::filesystem::path{module->getName()}.stem().stem()
            .concat("-" + std::string{entry->getFunctionReflection()->getName()}).concat(ext);
            auto cache = (stl::filesystem::instance().cache / file).concat(".mttcache");
            std::filesystem::create_directories(cache.parent_path());

            if ([&] {
                if (!std::filesystem::exists(cache)) return false;
                auto size = std::filesystem::file_size(cache);
                if (size != hash->getBufferSize()) return false;
                auto stream = std::ifstream{cache, std::ios::binary};
                auto buffer = std::vector<char>(size);
                stream.read(buffer.data(), buffer.size());
                return std::memcmp(buffer.data(), hash->getBufferPointer(), size) == 0;
            }()) return;
            auto version = std::ofstream{cache, std::ios::binary};
            version.write(view<char>(hash->getBufferPointer()), hash->getBufferSize());

            auto code = Slang::ComPtr<slang::IComponentType>{};
            auto unit = std::to_array<mut<slang::IComponentType>>({module, entry});
            print(session->createCompositeComponentType(
                unit.data(), unit.size(),
                code.writeRef(), diagnostic.writeRef())
            );

            auto linked = Slang::ComPtr<slang::IComponentType>{};
            auto kernel = Slang::ComPtr<slang::IBlob>{};
            print(code->link(linked.writeRef(), diagnostic.writeRef()));
            print(linked->getEntryPointCode(0, 0, kernel.writeRef(), diagnostic.writeRef()));

            auto compiled = std::ofstream{out / file, std::ios::binary};
            std::filesystem::create_directories((out / file).parent_path());
            compiled.write(view<char>(kernel->getBufferPointer()), kernel->getBufferSize());
        }

        auto build(
            cref<std::filesystem::path> dir,
            cref<std::filesystem::path> out
        ) noexcept -> void {
            this->dir = dir;
            this->out = out;

            auto targets = std::array<slang::TargetDesc, 1>{};
            targets.front().format = format;
            targets.front().forceGLSLScalarBufferLayout = true;
            auto paths = std::to_array({dir.c_str()});
            auto options = std::to_array<slang::CompilerOptionEntry>({
                {
                    .name = slang::CompilerOptionName::ForceCLayout,
                    .value = {.intValue0 = 1}
                }
            });

            slang::createGlobalSession(global_session.writeRef());
            global_session->createSession({
                .targets = targets.data(),
                .targetCount = targets.size(),
                .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
                .searchPaths = paths.data(),
                .searchPathCount = paths.size(),
                .compilerOptionEntries = options.data(),
                .compilerOptionEntryCount = options.size(),
            }, session.writeRef());

            for (auto& src: std::filesystem::recursive_directory_iterator(this->dir))
                if (src.path().string().ends_with(kernel)) {
                    auto module = session->loadModule(src.path().c_str());
                    for (auto i = 0; i < module->getDefinedEntryPointCount(); ++i)
                        compile(module, i, src);
                }
        }
    };

    Compiler::Compiler() noexcept {}

    auto Compiler::build(std::string_view dir, std::string_view out) noexcept -> void {
        return impl->build(dir, out);
    }
}
