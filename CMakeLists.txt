cmake_minimum_required(VERSION 3.20)
project(tsssni)

set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(engine main.cpp)
target_link_libraries(engine Vulkan::Vulkan glfw)
