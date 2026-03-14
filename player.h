#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"
#include "level.h"

#define PLAYER_W        24
#define PLAYER_H        32
#define PLAYER_SPEED   200.0f   /* pixels per second */
#define PLAYER_JUMP   -520.0f   /* initial jump velocity (negative = up) */
#define GRAVITY        900.0f   /* pixels per second squared */

typedef struct {
    float x, y;
    float vx, vy;
    bool  on_ground;
    bool  on_spring;   /* set true for one frame when spring launches */
    bool  alive;
} Player;

void player_init(Player *p, float start_x, float start_y);
void player_handle_input(Player *p, const uint8_t *keys,
                         SDL_GameController *ctrl);
void player_update(Player *p, const Level *lvl, float dt);
/* tex is a spritesheet with 2 frames side-by-side (idle | jump).
   Pass NULL to fall back to coloured rectangles. */
void player_render(const Player *p, SDL_Renderer *renderer,
                   SDL_Texture *tex);
AABB player_aabb(const Player *p);

#endif
