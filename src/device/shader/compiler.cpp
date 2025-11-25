#include <metatron/device/shader/compiler.hpp>
#include <metatron/core/stl/print.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <filesystem>
#include <fstream>

namespace mtt::shader {
    struct Compiler::Impl final {
        std::filesystem::path dir;
        std::filesystem::path out;
        Slang::ComPtr<slang::IGlobalSession> global_session;

        Impl() noexcept {
            slang::createGlobalSession(global_session.writeRef());
        }

        auto compile(cref<std::filesystem::path> src) noexcept -> void {
            #ifdef __linux__
                auto constexpr format = SLANG_SPIRV;
                auto constexpr ext = std::string_view{".spirv"};
            #elif __APPLE__
                auto constexpr format = SLANG_METAL_LIB;
                auto constexpr ext = std::string_view{".metallib"};
            #endif

            auto paths = std::to_array({dir.c_str()});
            auto targets = std::array<slang::TargetDesc, 1>{};
            auto& target = targets.front();
            target.format = format;
            target.forceGLSLScalarBufferLayout = true;

            auto session = Slang::ComPtr<slang::ISession>{};
            global_session->createSession({
                .targets = targets.data(),
                .targetCount = targets.size(),
                .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
                .searchPaths = paths.data(),
                .searchPathCount = paths.size(),
            }, session.writeRef());

            auto module = session->loadModule(src.stem().c_str());
            auto entry = Slang::ComPtr<slang::IEntryPoint>{};
            module->findEntryPointByName("main", entry.writeRef());

            auto program = Slang::ComPtr<slang::IComponentType>{};
            auto components = std::to_array<mut<slang::IComponentType>>({module, entry});
            session->createCompositeComponentType(components.data(), 2, program.writeRef());

            auto linked = Slang::ComPtr<slang::IComponentType>{};
            auto diagnostic = Slang::ComPtr<slang::IBlob>{};
            auto kernel = Slang::ComPtr<slang::IBlob>{};
            program->link(linked.writeRef(), diagnostic.writeRef());
            linked->getEntryPointCode(0, 0, kernel.writeRef(), diagnostic.writeRef());

            if (diagnostic && diagnostic->getBufferSize() > 0)
                std::println("{}", view<char>(diagnostic->getBufferPointer()));

            auto dst = out / src.stem().stem().concat(ext);
            auto file = std::ofstream{dst, std::ios::binary};
            file.write(view<char>(kernel->getBufferPointer()), kernel->getBufferSize());
        }

        auto build(
            cref<std::filesystem::path> dir,
            cref<std::filesystem::path> out
        ) noexcept -> void {
            this->dir = dir;
            this->out = out;
            for (auto& entry: std::filesystem::recursive_directory_iterator(this->dir)) {
                auto constexpr kernel_extension = std::string_view{".kernel.slang"};
                if (entry.path().string().ends_with(kernel_extension))
                    compile(entry.path());
            }
        }
    };

    Compiler::Compiler() noexcept {}

    auto Compiler::build(std::string_view dir, std::string_view out) noexcept -> void {
        return impl->build(dir, out);
    }
}
