#include <test/test.hpp>
#include <GLFW/glfw3.h>

auto main() -> int
{
  print_vulkan_version();
  std::cout << GLFW_TRUE << std::endl;
  return 0;
}
