cmake_minimum_required(VERSION 3.12)
project(Vulkan_Tutorials)

set(CMAKE_CXX_STANDARD 14)

link_libraries(libvulkan.so libSDL2.so libX11.so)

add_executable(1-test-vulkan-env 1-test-vulkan-env/main.cpp)
add_executable(2-start-sdl-window 2-start-sdl-window/main.cpp)
add_executable(3-start-xcb-window 3-start-xcb-window/main.cpp)
add_executable(4-init-vulkan-device 4-init-vulkan-device/main.cpp)
add_executable(5-init-vulkan-swap-chain 5-init-vulkan-swap-chain/main.cpp)