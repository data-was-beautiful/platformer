#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "level.h"
#include "player.h"
#include "enemy.h"
#include "lootbox.h"
#include "sprites.h"
#include "audio.h"
#include "title.h"

#define WINDOW_W        1280
#define WINDOW_H         640
#define STARTING_LIVES     3
#define MAX_LEVEL         99

typedef enum {
    STATE_TITLE   = 0,
    STATE_PLAYING = 1,
    STATE_QUIT    = 2
} AppState;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    AppState    state;
    TitleScreen title;

    Level          level;
    Player         player;
    Enemy          enemies[MAX_ENEMIES];
    int            enemy_count;
    LootBoxManager lootboxes;

    Sprites sprites;
    Audio   audio;

    int  current_level;
    int  score;
    int  lives;
    bool music_on;
} Game;

int  game_init(Game *g);
void game_handle_events(Game *g);
void game_update(Game *g, float dt);
void game_render(Game *g);
void game_shutdown(Game *g);

void game_start_playing(Game *g);
void game_goto_title(Game *g);
void game_load_level(Game *g, int level_num);

#endif
