#include <metatron/device/shader/compiler.hpp>
#include <metatron/core/stl/print.hpp>
#include <argparse/argparse.hpp>

using namespace mtt;

auto main(i32 argc, mut<char> argv[]) noexcept -> i32 {
    auto argparser = argparse::ArgumentParser{"metatron-builder", "0.2.0"};

    argparser.add_argument("-d", "--directory")
    .required().help("directory contains shaders")
    .default_value(std::string{"./"});

    argparser.add_argument("-o", "--output")
    .required().help("directory to dump compiled ir")
    .default_value(std::string{"./"});

    try {
        argparser.parse_args(argc, argv);
    } catch (cref<std::exception> err) {
        std::cout << argparser;
        stl::abort("argparser error: {}", err.what());
    }

    auto directory = argparser.get<std::string>("-d");
    auto output = argparser.get<std::string>("-o");
    shader::Compiler::instance().build(directory, output);
    return 0;
}
