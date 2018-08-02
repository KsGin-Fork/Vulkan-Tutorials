//
// Created by ksgin on 18-8-2.
//
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <dlfcn.h>

const int width = 800;
const int height = 600;

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("load vulkan lib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        return -1;
    }

    SDL_ShowWindow(window);

    SDL_Event event;
    bool isQuit = false;
    while (!isQuit) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            isQuit = true;
        }
    }

    SDL_DestroyWindow(window);
}