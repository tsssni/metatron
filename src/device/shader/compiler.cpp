#include <metatron/device/shader/compiler.hpp>
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <filesystem>
#include <fstream>
#include <cstring>

namespace mtt::shader {
    auto constexpr format = SLANG_SPIRV;
    auto constexpr ext = std::string_view{".spirv"};
    auto constexpr kernel = std::string_view{".kernel.slang"};

    struct Compiler::Impl final {
        std::filesystem::path dir;
        std::filesystem::path out;
        Slang::ComPtr<slang::IGlobalSession> global_session;
        Slang::ComPtr<slang::ISession> session;

        Impl() noexcept {}

        auto compile(
            mut<slang::IModule> module, i32 idx,
            cref<std::filesystem::path> src
        ) noexcept -> void {
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
            .concat("." + std::string{entry->getFunctionReflection()->getName()}).concat(ext);
            auto cache = (stl::filesystem::instance().cache / file).concat(".mttcache");
            std::filesystem::create_directories(cache.parent_path());

            if (false && [&] {
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

            auto layout = reflect(linked->getLayout(0), file);
            auto reflection = (file.parent_path() / file.stem()).concat(".json");
            auto serialized = std::string{};
            if (auto e = glz::write_json(layout, serialized); e)
                std::println(
                    "write pipeline layout {} with glaze error {}",
                    reflection.c_str(), glz::format_error(e)
                );
            auto reflected = std::ofstream{out / reflection};
            std::filesystem::create_directories((out / reflection).parent_path());
            reflected.write(serialized.c_str(), serialized.size());

        #if __APPLE__
            cross({view<byte>(kernel->getBufferPointer()), kernel->getBufferSize()});
        #else
            auto compiled = std::ofstream{out / file, std::ios::binary};
            std::filesystem::create_directories((out / file).parent_path());
            compiled.write(view<char>(kernel->getBufferPointer()), kernel->getBufferSize());
        #endif
        }

        auto cross(std::span<byte const> kernel) noexcept -> void {
            auto compiler = spirv_cross::CompilerMSL{view<u32>(kernel.data()), kernel.size() / sizeof(u32)};
            auto options = spirv_cross::CompilerMSL::Options{};
            options.set_msl_version(4, 0);
            compiler.set_msl_options(options);
            std::println("{}", compiler.compile());
        }

        auto reflect(
            mut<slang::ShaderReflection> reflection,
            cref<std::filesystem::path> file
        ) noexcept -> Layout {
            if (!reflection) {
                std::println("failed to generate reflection");
                std::abort();
            }
            auto serialized = shader::Layout{};

            auto parse_resource = [](mut<slang::TypeLayoutReflection> t) {
                using Type = SlangResourceShape;
                using Access = SlangResourceAccess;
                auto type = Descriptor::Type{};
                auto access = Descriptor::Access{};

                switch (t->getResourceShape()) {
                case SLANG_TEXTURE_2D: type = Descriptor::Type::image; break;
                case SLANG_TEXTURE_3D: type = Descriptor::Type::grid; break;
                case SLANG_ACCELERATION_STRUCTURE: type = Descriptor::Type::accel; break;
                default:
                    std::println("descriptor type {} not support", i32(t->getResourceShape()));
                    std::abort();
                }

                switch (t->getResourceAccess()) {
                case SLANG_RESOURCE_ACCESS_NONE:
                case SLANG_RESOURCE_ACCESS_READ: access = Descriptor::Access::readonly; break;
                case SLANG_RESOURCE_ACCESS_READ_WRITE: access = Descriptor::Access::writeonly; break;
                case SLANG_RESOURCE_ACCESS_WRITE: access = Descriptor::Access::writeonly; break;
                default:
                    std::println("descriptor access {} not supported", i32(t->getResourceAccess()));
                    std::abort();
                }

                return std::make_tuple(type, access);
            };

            auto parse_type = [&serialized, &parse_resource](
                this auto self,
                mut<slang::TypeLayoutReflection> t,
                std::string p = "",
                i32 s = 0
            ) -> void {
                for (auto i = 0; i < t->getFieldCount(); ++i) {
                    using Kind = slang::TypeReflection::Kind;
                    using Unit = slang::ParameterCategory;
                    auto Index = Unit::DescriptorTableSlot;
                    auto Set = Unit::SubElementRegisterSpace;
                    auto Size = Unit::Uniform;

                    auto field = t->getFieldByIndex(i);
                    auto type = field->getTypeLayout();
                    auto element = type->getElementTypeLayout();
                    auto kind = field->getType()->getKind();
                    auto name = field->getName();

                    auto table = kind == Kind::ParameterBlock;
                    auto var = table ? type->getContainerVarLayout() : field;
                    auto desc = Descriptor{};

                    auto constexpr base = 1;
                    desc.path = p + (p.size() == 0 ? "" : ".") + name;
                    desc.set = table ? base + field->getOffset(Set) : s;
                    for (auto j = 0; j < var->getCategoryCount(); ++j)
                        if (var->getCategoryByIndex(j) == Index)
                            desc.index = var->getOffset(Index);

                    if (table && desc.index >= 0) {
                        desc.type = Descriptor::Type::parameter;
                        desc.size = element->getSize();
                    } else if (kind == Kind::SamplerState) {
                        desc.type = Descriptor::Type::sampler;
                    } else if (kind == Kind::Resource) {
                        auto [t, a] = parse_resource(type);
                        desc.type = t; desc.access = a;
                    } else if (kind == Kind::Array) {
                        desc.size = type->getElementCount();
                        if (desc.size == 0) desc.size = -1; // bindless
                        auto [t, a] = parse_resource(type->getElementTypeLayout());
                        desc.type = t; desc.access = a;
                    }

                    if (desc.index >= 0) serialized.descriptors.push_back(desc);
                    self(table ? element : type, desc.path, desc.set);
                }
            };

            parse_type(reflection->getGlobalParamsTypeLayout());
            parse_type(reflection->getEntryPointByIndex(0)->getTypeLayout());
            std::ranges::sort(serialized.descriptors, [](auto&& x, auto&& y) {
                return x.set < y.set || (x.set == y.set && x.index < y.index);
            });
            return serialized;
        }

        auto build(
            cref<std::filesystem::path> dir,
            cref<std::filesystem::path> out
        ) noexcept -> void {
            this->dir = dir;
            this->out = out;

            using Option = slang::CompilerOptionName;
            auto int_opt = [](Option opt, i32 v) {
                return slang::CompilerOptionEntry{
                    .name = opt, .value = {.intValue0 = v}
                };
            };
            auto str_opt = [](Option opt, std::string_view v) {
                return slang::CompilerOptionEntry{
                    .name = opt, .value = {.stringValue0 = v.data()}
                };
            };

            auto target = slang::TargetDesc{.format = format};
            auto targets = std::to_array({target});
            auto paths = std::to_array({dir.c_str()});
            auto options = std::to_array<slang::CompilerOptionEntry>({
                int_opt(Option::ForceCLayout, 1),
                int_opt(Option::Optimization, SlangOptimizationLevel::SLANG_OPTIMIZATION_LEVEL_MAXIMAL),
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
