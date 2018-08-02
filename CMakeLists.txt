cmake_minimum_required(VERSION 3.12)
project(Vulkan_Tutorials)

set(CMAKE_CXX_STANDARD 14)

link_directories(/usr/lib)

link_libraries(libvulkan.so libSDL2.so)

add_executable(1-test-vulkan-env 1-test-vulkan-env/main.cpp)