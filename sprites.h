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
 */

typedef struct {
    SDL_Texture *player;
    SDL_Texture *enemy;
    SDL_Texture *tile;
    SDL_Texture *spring;
    SDL_Texture *lootbox;
    SDL_Texture *boss;
} Sprites;

void sprites_load(Sprites *s, SDL_Renderer *renderer);
void sprites_free(Sprites *s);

#endif
