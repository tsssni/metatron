#include <metatron/render/scene/args.hpp>
#include <metatron/core/stl/print.hpp>
#include <argparse/argparse.hpp>

namespace mtt::scene {
    auto Args::parse(i32 argc, mut<char> argv[]) -> void {
        auto argparser = argparse::ArgumentParser{"metatron-tracer", "0.2.0"};

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
        } catch (cref<std::exception> err) {
            std::println("argparser error: {}", err.what());
            std::cout << argparser;
            std::abort();
        }
        scene = argparser.get<std::string>("-s");
        output = argparser.get<std::string>("-o");
        address = argparser.get<std::string>("-a");
    }
}
