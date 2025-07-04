#include <algorithm>
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

struct RGB {
    Uint8 r, g, b;
};

RGB valueToRGB(float value) {
    // Simples: branco → azul → vermelho
    if (value < 0.5f)
        return {0, static_cast<Uint8>(value * 2 * 255), 255};
    else
        return {static_cast<Uint8>((value - 0.5f) * 2 * 255), 0, 255 - static_cast<Uint8>((value - 0.5f) * 2 * 255)};
}

int main() {
    const int size = 256;
    std::vector<long> heat(size * size);

    // Preenche com valores simulados
    for (int i = 0; i < size * size; ++i)
        heat[i] = rand() % 1000;

    // Normaliza
    long minVal = *std::min_element(heat.begin(), heat.end());
    long maxVal = *std::max_element(heat.begin(), heat.end());

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init Error\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Heatmap SDL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        size, size, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
        size, size);

    std::vector<Uint8> pixels(size * size * 3);

    for (int i = 0; i < size * size; ++i) {
        float norm = float(heat[i] - minVal) / float(maxVal - minVal + 1e-5f);
        RGB color = valueToRGB(norm);
        pixels[i * 3 + 0] = color.r;
        pixels[i * 3 + 1] = color.g;
        pixels[i * 3 + 2] = color.b;
    }

    SDL_UpdateTexture(texture, nullptr, pixels.data(), size * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    // Espera fechar
    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}