#include <metatron/render/scene/args.hpp>
#include <metatron/render/scene/scene.hpp>
#include <metatron/core/stl/print.hpp>
#include <argparse/argparse.hpp>

using namespace mtt;

auto main(i32 argc, mut<char> argv[]) -> i32 {
    auto argparser = argparse::ArgumentParser{"metatron-tracer", "0.2.0"};

    argparser.add_argument("-s", "--scene")
    .required().help("directory contains scene.json")
    .default_value(std::string{"./"});

    argparser.add_argument("-o", "--output")
    .required().help("result image path in exr format")
    .default_value(std::string{"./result.exr"});

    argparser.add_argument("-d", "--device")
    .required().help("device to run renderer")
    .default_value(std::string{"cpu"})
    .choices("cpu", "gpu");


    argparser.add_argument("-a", "--address")
    .required().help("address to tev server")
    .default_value(std::string{""});

    try {
        argparser.parse_args(argc, argv);
    } catch (cref<std::exception> err) {
        std::cout << argparser;
        stl::abort("argparser error: {}", err.what());
    }

    auto& args = scene::Args::instance();
    args.scene = argparser.get<std::string>("-s");
    args.output = argparser.get<std::string>("-o");
    args.address = argparser.get<std::string>("-a");
    args.device = argparser.get<std::string>("-d");
    scene::run();

    return 0;
}
