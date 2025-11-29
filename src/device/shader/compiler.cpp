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
    template<typename T>
    using com = Slang::ComPtr<T>;

    struct Compiler::Impl final {
        std::filesystem::path dir;
        std::filesystem::path out;
        Layout layout;
        com<slang::IGlobalSession> global_session;
        com<slang::ISession> session;
        com<slang::IBlob> diagnostic;

        Impl() noexcept {}

        auto guard(SlangResult result) {
            if (SLANG_SUCCEEDED(result)) return;
            std::println("slang compilation failed with {}", result);
            std::abort();
        }
        auto diagnose() {
            if (!diagnostic || diagnostic->getBufferSize() == 0) return;
            std::println("{}", view<char>(diagnostic->getBufferPointer()));
        }
        auto diagnose(SlangResult result) { diagnose(); guard(result); }
        auto diagnose(auto* ptr) { diagnose(); if (!ptr) std::abort(); return ptr; }

        auto compile(
            mut<slang::IModule> module, i32 idx,
            cref<std::filesystem::path> src
        ) noexcept -> void {
            auto entry = com<slang::IEntryPoint>{};
            auto hash = com<slang::IBlob>{};
            guard(module->getDefinedEntryPoint(idx, entry.writeRef()));
            module->getEntryPointHash(idx, 0, hash.writeRef());
            if (!hash || hash->getBufferSize() == 0) {
                std::println(
                    "slang failed to generate hash for {} {}",
                    src.c_str(), entry->getFunctionReflection()->getName()
                );
                std::abort();
            }

            auto path = src.parent_path() / src.stem().stem()
            .concat("." + std::string{entry->getFunctionReflection()->getName()});
            auto cache = (stl::filesystem::instance().cache / "shader" / path).concat(".mttcache");
            std::filesystem::create_directories((out / path).parent_path());
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

            auto code = com<slang::IComponentType>{};
            auto unit = std::to_array<mut<slang::IComponentType>>({module, entry});
            diagnose(session->createCompositeComponentType(
                unit.data(), unit.size(),
                code.writeRef(), diagnostic.writeRef())
            );

            auto linked = com<slang::IComponentType>{};
            auto kernel = com<slang::IBlob>{};
            diagnose(code->link(linked.writeRef(), diagnostic.writeRef()));
            diagnose(linked->getEntryPointCode(0, 0, kernel.writeRef(), diagnostic.writeRef()));

            auto layout = reflect(linked->getLayout(0), path);
        #if __APPLE__
            cross({view<byte>(kernel->getBufferPointer()), kernel->getBufferSize()}, layout);
        #else
            auto compiled = std::ofstream{(out / path).concat(".spirv"), std::ios::binary};
            compiled.write(view<char>(kernel->getBufferPointer()), kernel->getBufferSize());
        #endif
        }

        auto cross(
            std::span<byte const> kernel,
            cref<Layout> layout
        ) noexcept -> void {
            auto compiler = spirv_cross::CompilerMSL{view<u32>(kernel.data()), kernel.size() / sizeof(u32)};

            using Options = spirv_cross::CompilerMSL::Options;
            auto options = Options{};
            options.set_msl_version(4, 0);
            options.argument_buffers = true;
            options.argument_buffers_tier = Options::ArgumentBuffersTier::Tier2;
            options.enable_decoration_binding = true;
            compiler.set_msl_options(options);

            for (auto i = 0; i <= layout.descriptors.back().set; i++)
                compiler.set_argument_buffer_device_address_space(i, true);
            std::println("{}", compiler.compile());
        }

        auto reflect(
            mut<slang::ShaderReflection> reflection,
            cref<std::filesystem::path> path
        ) noexcept -> Layout {
            if (!reflection) {
                std::println("failed to generate reflection");
                std::abort();
            }

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

            auto parse_type = [&parse_resource](
                this auto self,
                mut<slang::TypeLayoutReflection> t,
                ref<Layout> layout,
                std::string path = "",
                i32 set = 0
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
                    desc.path = path + (path.size() == 0 ? "" : ".") + name;
                    desc.set = table ? base + field->getOffset(Set) : set;
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

                    if (desc.index >= 0) layout.descriptors.push_back(desc);
                    self(table ? element : type, layout, desc.path, desc.set);
                }
            };

            auto serialize = [this](ref<Layout> layout, std::string_view path) {
                std::ranges::sort(this->layout.descriptors, [](auto&& x, auto&& y) {
                    return x.set < y.set || (x.set == y.set && x.index < y.index);
                });
                auto serialized = std::string{};
                if (auto e = glz::write_json(layout, serialized); e)
                    std::println(
                        "write pipeline layout {} with glaze error {}",
                        path.data(), glz::format_error(e)
                    );
                auto reflected = std::ofstream{(out / path).concat(".json")};
                reflected.write(serialized.c_str(), serialized.size());
            };

            if (layout.descriptors.empty()) {
                parse_type(reflection->getGlobalParamsTypeLayout(), this->layout);
                serialize(this->layout, "metatron");
            }
            Layout layout;
            parse_type(reflection->getEntryPointByIndex(0)->getTypeLayout(), layout);
            serialize(layout, path.c_str());
            return layout;
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

            auto target = slang::TargetDesc{.format = SLANG_SPIRV};
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
                if (src.path().string().ends_with(".kernel.slang")) {
                    auto module = diagnose(session->loadModule(src.path().c_str(), diagnostic.writeRef()));
                    auto path = std::filesystem::relative(src, this->dir);
                    for (auto i = 0; i < module->getDefinedEntryPointCount(); ++i)
                        compile(module, i, path);
                }
        }
    };

    Compiler::Compiler() noexcept {}

    auto Compiler::build(std::string_view dir, std::string_view out) noexcept -> void {
        return impl->build(dir, out);
    }
}
