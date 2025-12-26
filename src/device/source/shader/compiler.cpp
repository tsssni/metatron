#include <metatron/device/shader/compiler.hpp>
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/json.hpp>
#include <metatron/core/stl/print.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <slang-com-ptr.h>
#include <slang.h>

namespace mtt::shader {
    struct Layout final {
        std::vector<std::string> names;
        std::vector<Set> sets;
    };

    template<typename T>
    using com = Slang::ComPtr<T>;

    struct Compiler::Impl final {
        std::filesystem::path dir;
        std::filesystem::path out;
        std::unordered_map<std::string, std::string> cache;

        com<slang::IGlobalSession> global_session;
        com<slang::ISession> session;
        com<slang::IBlob> diagnostic;

        auto guard(SlangResult result) {
            if (SLANG_SUCCEEDED(result)) return;
            stl::abort("slang compilation failed with {}", result);
        }
        auto diagnose() {
            if (!diagnostic || diagnostic->getBufferSize() == 0) return;
            stl::print("{}", view<char>(diagnostic->getBufferPointer()));
        }
        auto diagnose(SlangResult result) { diagnose(); guard(result); }
        auto diagnose(auto* ptr) { diagnose(); if (!ptr) stl::abort(); return ptr; }

        auto compile(
            mut<slang::IModule> module, i32 idx,
            cref<std::filesystem::path> src
        ) noexcept -> void {
            auto entry = com<slang::IEntryPoint>{};
            auto hash = com<slang::IBlob>{};
            guard(module->getDefinedEntryPoint(idx, entry.writeRef()));
            module->getEntryPointHash(idx, 0, hash.writeRef());
            if (!hash || hash->getBufferSize() == 0)
                stl::abort(
                    "slang failed to generate hash for {} {}",
                    src.c_str(), entry->getFunctionReflection()->getName()
                );

            auto path = src.parent_path() / src.stem().stem()
            .concat("." + std::string{entry->getFunctionReflection()->getName()});
            std::filesystem::create_directories((out / path).parent_path());

            if ([&] {
                auto abs = std::filesystem::absolute(path);
                auto hex = std::string{}; hex.reserve(hash->getBufferSize() * 2);
                for (auto i = 0; i < hash->getBufferSize(); i++)
                    hex += std::format("{:02x}", view<byte>(hash->getBufferPointer())[i]);
                if (cache.contains(abs) && cache[abs] == hex) return true;
                cache[abs] = hex; return false;
            }()) return;

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
            auto spirv = std::span{view<byte>(kernel->getBufferPointer()), kernel->getBufferSize()};
            cross(spirv, layout, path);
        }

        auto cross(
            std::span<byte const> kernel,
            cref<Layout> layout,
            cref<std::filesystem::path> path
        ) noexcept -> void {
        #ifdef MTT_SYSTEM_DARWIN
            auto compiler = spirv_cross::CompilerMSL{view<u32>(kernel.data()), kernel.size() / sizeof(u32)};

            using Options = spirv_cross::CompilerMSL::Options;
            auto options = Options{};
            options.set_msl_version(4, 0);
            options.argument_buffers = true;
            options.argument_buffers_tier = Options::ArgumentBuffersTier::Tier2;
            options.enable_decoration_binding = true;
            compiler.set_msl_options(options);

            for (auto i = 0; i <= layout.sets.size(); i++)
                compiler.set_argument_buffer_device_address_space(i, true);
            auto metal = compiler.compile();
            auto compiled = std::ofstream{(out / path).concat(".metal")};
            compiled.write(metal.c_str(), metal.size());
            stl::filesystem::store((out/ path).concat(".metal"), metal);
        #else
            auto spirv = (out / path).concat(".spirv");
            stl::filesystem::store(spirv, kernel, std::ios::binary);
        #endif
        }

        auto reflect(
            mut<slang::ShaderReflection> reflection,
            cref<std::filesystem::path> path
        ) noexcept -> Layout {
            if (!reflection) stl::abort("failed to generate reflection");

            auto parse_resource = [](
                mut<slang::TypeLayoutReflection> t,
                ref<Descriptor> desc
            ) {
                using Type = Descriptor::Type;
                using Access = Descriptor::Access;

                switch (t->getResourceShape()) {
                case SLANG_TEXTURE_2D: desc.type = Type::image; break;
                case SLANG_TEXTURE_3D: desc.type = Type::grid; break;
                case SLANG_ACCELERATION_STRUCTURE: desc.type = Type::accel; break;
                default:
                    stl::abort("descriptor type {} not support", i32(t->getResourceShape()));
                }

                switch (t->getResourceAccess()) {
                case SLANG_RESOURCE_ACCESS_NONE:
                case SLANG_RESOURCE_ACCESS_READ: desc.access = Access::readonly; break;
                case SLANG_RESOURCE_ACCESS_READ_WRITE: desc.access = Access::readwrite; break;
                default:
                    stl::abort("descriptor access {} not supported", i32(t->getResourceAccess()));
                }
            };

            auto parse_var = [&parse_resource](
                this auto&& self,
                mut<slang::VariableLayoutReflection> reflection,
                ref<Layout> layout,
                std::string path = "",
                u32 block = 0,
                bool parameter = false
            ) -> void {
                auto parse_type = [&] (mut<slang::TypeLayoutReflection> reflection) {
                    for (auto i = 0; i < reflection->getFieldCount(); ++i) {
                        auto field = reflection->getFieldByIndex(i);
                        self(field, layout, path, block, parameter);
                    }
                };

                using Kind = slang::TypeReflection::Kind;
                using Unit = slang::ParameterCategory;
                auto Index = Unit::DescriptorTableSlot;
                auto Set = Unit::SubElementRegisterSpace;
                auto Size = Unit::Uniform;

                auto type = reflection->getTypeLayout();
                auto element = type->getElementTypeLayout();
                auto kind = reflection->getType()->getKind();
                auto name = reflection->getName();

                auto table = kind == Kind::ParameterBlock;
                auto var = table ? type->getContainerVarLayout() : reflection;
                auto desc = Descriptor{};

                auto field = path + (path.size() == 0 ? "" : ".") + (name ? name : "");
                auto set = table ? reflection->getOffset(Set) : block;
                auto index = math::maxv<u32>;
                for (auto j = 0; j < var->getCategoryCount(); ++j)
                    if (var->getCategoryByIndex(j) == Index)
                        index = var->getOffset(Index);

                if (layout.sets.size() <= set) {
                    layout.sets.resize(set + 1);
                    layout.names.resize(set + 1);
                }
                if (table) layout.names[set] = name;
                if (index != math::maxv<u32> && layout.sets[set].size() <= index)
                    layout.sets[set].resize(index + 1);
                if (index == math::maxv<u32>) {
                    path = field; block = set;
                    parse_type(table ? element : type);
                    return;
                }

                switch (kind) {
                case Kind::ParameterBlock:
                    desc.type = Descriptor::Type::parameter;
                    desc.size = element->getSize();
                    if (!parameter) parameter = true;
                    else stl::abort("embedded parameter block not allowed");
                    break;
                case Kind::SamplerState:
                    desc.type = Descriptor::Type::sampler;
                    break;
                case Kind::Resource:
                    parse_resource(type, desc);
                    break;
                case Kind::Array:
                    desc.size = type->getElementCount();
                    if (desc.size == 0) desc.size = -1; // bindless
                    parse_resource(type->getElementTypeLayout(), desc);
                    break;
                default: break;
                }

                desc.path = field;
                layout.sets[set][index] = desc;
                path = field; block = set;
                parse_type(table ? element : type);
            };

            auto global = Layout{};
            parse_var(reflection->getGlobalParamsVarLayout(), global);
            for (auto i = 0; i < global.sets.size(); ++i) {
                auto index = global.sets.size() > 1 ? "-" + std::to_string(i) : "";
                auto postfix = ".global" + index + ".json";
                stl::json::store((out / path.stem()).concat(postfix), global.sets[i]);
            }

            auto layout = Layout{};
            parse_var(reflection->getEntryPointByIndex(0)->getVarLayout(), layout);
            for (auto i = 0; i < layout.sets.size(); ++i) {
                auto postfix = "." + layout.names[i] + ".json";
                stl::json::store((out / path).concat(postfix), layout.sets[i]);
            }

            auto merged = global;
            std::ranges::copy(layout.names, std::back_inserter(merged.names));
            std::ranges::copy(layout.sets, std::back_inserter(merged.sets));
            return merged;
        }

        auto build(
            cref<std::filesystem::path> dir,
            cref<std::filesystem::path> out
        ) noexcept -> void {
            this->dir = dir;
            this->out = out;

            auto database = this->out / "cache.json";
            if (std::filesystem::exists(database))
                stl::json::load(database, cache);

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
                int_opt(Option::VulkanUseEntryPointName, 1),
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
            stl::json::store(database, cache);
        }
    };

    Compiler::Compiler() noexcept {}

    auto Compiler::build(std::string_view dir, std::string_view out) noexcept -> void {
        return impl->build(dir, out);
    }
}
