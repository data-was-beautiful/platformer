#include <stdio.h>
#include <SDL2/SDL.h>
#include "game.h"

int main(void) {
    Game g;

    if (game_init(&g) != 0) {
        fprintf(stderr, "Failed to initialise game.\n");
        return 1;
    }

    const float  TARGET_DT   = 1.0f / 60.0f;
    float        accumulator = 0.0f;
    uint64_t     prev_ticks  = SDL_GetTicks64();

    while (g.state != STATE_QUIT) {
        uint64_t now     = SDL_GetTicks64();
        float    elapsed = (float)(now - prev_ticks) / 1000.0f;
        prev_ticks = now;
        accumulator += elapsed;

        game_handle_events(&g);
        if (g.state == STATE_QUIT) break;

        while (accumulator >= TARGET_DT) {
            game_update(&g, TARGET_DT);
            accumulator -= TARGET_DT;
        }

        game_render(&g);
    }

    game_shutdown(&g);
    return 0;
}
