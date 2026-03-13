#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>

/*
 * Sprites module — loads PNG textures via SDL2_image.
 * All textures are optional: if a file is missing, the pointer stays NULL
 * and callers fall back to SDL_RenderFillRect.
 *
 * Expected files in assets/:
 *   assets/player.png   — 24x32, frame 0 = idle, frame 1 = jump
 *   assets/enemy.png    — 24x24, single frame
 *   assets/tile.png     — 32x32, single tile
 */

typedef struct {
    SDL_Texture *player;   /* spritesheet: 2 frames side-by-side (48x32) */
    SDL_Texture *enemy;    /* single frame 24x24                          */
    SDL_Texture *tile;     /* single frame 32x32                          */
} Sprites;

/* Load all textures. Logs warnings for missing files but does not fail. */
void sprites_load(Sprites *s, SDL_Renderer *renderer);

/* Free all textures. */
void sprites_free(Sprites *s);

#endif
