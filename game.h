#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "level.h"
#include "player.h"
#include "enemy.h"

#define WINDOW_W   (LEVEL_COLS * TILE_SIZE)   /* 1280 */
#define WINDOW_H   (LEVEL_ROWS * TILE_SIZE)   /*  640 */

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    Level  level;
    Player player;
    Enemy  enemies[MAX_ENEMIES];
    int    enemy_count;

    bool   running;
    int    deaths;       /* simple score */
} Game;

int  game_init(Game *g);
void game_handle_events(Game *g);
void game_update(Game *g, float dt);
void game_render(Game *g);
void game_shutdown(Game *g);

#endif
