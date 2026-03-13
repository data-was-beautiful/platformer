#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "level.h"
#include "player.h"
#include "enemy.h"
#include "sprites.h"

#define WINDOW_W   1280
#define WINDOW_H    640

/* Path to the level file. If not found, falls back to the built-in map. */
#define LEVEL_FILE  "assets/level1.txt"

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    Level   level;
    Player  player;
    Enemy   enemies[MAX_ENEMIES];
    int     enemy_count;

    Sprites sprites;

    bool   running;
    int    deaths;
} Game;

int  game_init(Game *g);
void game_handle_events(Game *g);
void game_update(Game *g, float dt);
void game_render(Game *g);
void game_shutdown(Game *g);

#endif
