//
// Created by ksgin on 18-8-3.
//

#include <SDL2/SDL.h>
#include <iostream>
#include <assert.h>

const int screenWidth = 1024, screenHeight = 768;
char *windowTitle = "SDL 窗口";
SDL_Window *window;


int main() {
    assert(SDL_Init(SDL_INIT_EVERYTHING) != -1);
    window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                              SDL_WINDOW_RESIZABLE);
    assert(window);

    SDL_ShowWindow(window);

    SDL_Event event;
    bool isQuit = false;
    while (!isQuit) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) isQuit = true;
    }

    SDL_DestroyWindow(window);
}