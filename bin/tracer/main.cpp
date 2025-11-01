#include <metatron/render/scene/scene.hpp>
#include <metatron/core/stl/print.hpp>
#include <argparse/argparse.hpp>

using namespace mtt;

auto main(i32 argc, mut<char> argv[]) -> int {
    auto argparser = argparse::ArgumentParser{"metatron-tracer", "0.0.1"};

    argparser.add_argument("-s", "--scene")
    .required()
    .help("directory contains scene.json")
    .default_value(std::string{"./"});

    argparser.add_argument("-o", "--output")
    .required()
    .help("result image path in exr format")
    .default_value(std::string{"./result.exr"});

    argparser.add_argument("-a", "--address")
    .required()
    .help("address to tev server")
    .default_value(std::string{""});

    try {
        argparser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::println("argparser error: {}", err.what());
        std::cout << argparser;
        return 1;
    }

    scene::init();
    scene::test();
    return 0;
}
