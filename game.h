#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "level.h"
#include "player.h"
#include "enemy.h"
#include "sprites.h"
#include "audio.h"
#include "title.h"

#define WINDOW_W        1280
#define WINDOW_H         640
#define STARTING_LIVES     3
#define MAX_LEVEL         99   /* level files level1.txt … level99.txt */

/* Top-level game state */
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

    Level  level;
    Player player;
    Enemy  enemies[MAX_ENEMIES];
    int    enemy_count;

    Sprites sprites;
    Audio   audio;

    int  current_level;   /* 1-based, matches levelN.txt            */
    int  score;           /* +1 per stomp                           */
    int  lives;           /* starts at STARTING_LIVES, game over=0  */
    bool music_on;        /* persists across title/play transitions  */
} Game;

/* One-time SDL + window + renderer + asset init */
int  game_init(Game *g);

/* Per-frame */
void game_handle_events(Game *g);
void game_update(Game *g, float dt);
void game_render(Game *g);

/* Clean shutdown */
void game_shutdown(Game *g);

/* Internal helpers exposed so main.c stays thin */
void game_start_playing(Game *g);   /* enter STATE_PLAYING       */
void game_goto_title(Game *g);      /* return to STATE_TITLE      */
void game_load_level(Game *g, int level_num);  /* load levelN.txt */

#endif
