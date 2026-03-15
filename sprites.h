#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>

/*
 * Expected files in assets/:
 *   player.png   — 48×32  (2-frame spritesheet)
 *   enemy.png    — 24×24  (single frame)
 *   tile.png     — 32×32  (solid tile)
 *   spring.png   — 32×32  (spring tile)
 *   lootbox.png  — 24×24  (loot box)
 *   boss.png     — 40×40  (boss enemy, single frame)
 *   bg1.png      — any size, stretched to fill window (level 1 background)
 *   bg2.png      — level 2 background (bg3.png, bg4.png … etc.)
 *                  Falls back to a solid colour if the file is absent.
 */

typedef struct {
    SDL_Texture *player;
    SDL_Texture *enemy;
    SDL_Texture *tile;
    SDL_Texture *spring;
    SDL_Texture *lootbox;
    SDL_Texture *boss;
    SDL_Texture *bg;        /* current level background — swapped per level */
} Sprites;

void sprites_load(Sprites *s, SDL_Renderer *renderer);
void sprites_free(Sprites *s);

/* Load (or reload) the background for the given level number.
   Frees any previously-loaded background first.
   renderer is needed only when SDL2_image is compiled in. */
void background_load(Sprites *s, SDL_Renderer *renderer, int level_num);
void background_free(Sprites *s);

#endif
