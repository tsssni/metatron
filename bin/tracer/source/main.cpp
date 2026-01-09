#include <metatron/render/scene/args.hpp>
#include <metatron/render/scene/scene.hpp>
#include <metatron/core/stl/print.hpp>
#include <argparse/argparse.hpp>

using namespace mtt;

auto main(i32 argc, mut<char> argv[]) -> i32 {
    auto argparser = argparse::ArgumentParser{"metatron-tracer", "0.2.0"};

    argparser.add_argument("-s", "--scene")
    .help("directory contains scene.json")
    .default_value(std::string{"./"})
    .nargs(1).metavar("DIR");

    argparser.add_argument("-o", "--output")
    .help("result exr image path")
    .default_value(std::string{"./result.exr"})
    .nargs(1).metavar("PATH");

    argparser.add_argument("-d", "--device")
    .help("device to run renderer")
    .choices("cpu", "gpu")
    .default_value(std::string{"cpu"})
    .nargs(1).metavar("{cpu, gpu}");

    argparser.add_argument("-a", "--address")
    .help("address to tev server")
    .default_value(std::string{""})
    .nargs(1).metavar("ADDR");

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
