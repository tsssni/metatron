#pragma once
#include <vulkan/vulkan_hpp_macros.hpp>
#include <iostream>

auto print_vulkan_version() -> void
{
  std::cout << VULKAN_HPP_CPP_VERSION << std::endl;
}
