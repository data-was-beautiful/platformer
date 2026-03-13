#include "game.h"
#include "physics.h"
#include <stdio.h>

/* -------------------------------------------------------------------------
   Helpers
   ------------------------------------------------------------------------- */
static void spawn_enemies(Game *g) {
    g->enemy_count = g->level.enemy_spawn_count;
    if (g->enemy_count > MAX_ENEMIES) g->enemy_count = MAX_ENEMIES;
    for (int i = 0; i < g->enemy_count; i++)
        enemy_init(&g->enemies[i],
                   g->level.enemy_spawns[i].x,
                   g->level.enemy_spawns[i].y);
}

/* -------------------------------------------------------------------------
   Public API
   ------------------------------------------------------------------------- */
int game_init(Game *g) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    g->window = SDL_CreateWindow(
        "C Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!g->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return -1;
    }

    g->renderer = SDL_CreateRenderer(
        g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!g->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return -1;
    }

    /* Load level — fall back to built-in map if file is missing */
    if (!level_load(&g->level, LEVEL_FILE)) {
        fprintf(stderr, "game_init: using built-in map\n");
        level_init_builtin(&g->level);
    }

    /* Load sprites (graceful: NULL textures if files are absent) */
    sprites_load(&g->sprites, g->renderer);

    /* Spawn player and enemies from level data */
    player_init(&g->player,
                g->level.player_spawn.x,
                g->level.player_spawn.y);
    spawn_enemies(g);

    g->running = true;
    g->deaths  = 0;
    return 0;
}

void game_handle_events(Game *g) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            g->running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            g->running = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
            player_init(&g->player,
                        g->level.player_spawn.x,
                        g->level.player_spawn.y);
            spawn_enemies(g);
            g->deaths = 0;
        }
    }

    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    player_handle_input(&g->player, keys);
}

void game_update(Game *g, float dt) {
    if (dt > 0.05f) dt = 0.05f;

    player_update(&g->player, &g->level, dt);

    for (int i = 0; i < g->enemy_count; i++)
        enemy_update(&g->enemies[i], &g->level, dt);

    /* Player-enemy collision */
    AABB pbox = player_aabb(&g->player);
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        AABB ebox = enemy_aabb(&g->enemies[i]);
        if (aabb_overlap(pbox, ebox)) {
            float player_bottom = g->player.y + PLAYER_H;
            float enemy_top     = g->enemies[i].y;
            if (g->player.vy > 0 && player_bottom - enemy_top < 16) {
                g->enemies[i].alive = false;
                g->player.vy = PLAYER_JUMP * 0.6f;
            } else {
                g->deaths++;
                player_init(&g->player,
                            g->level.player_spawn.x,
                            g->level.player_spawn.y);
            }
        }
    }
}

static void draw_hud(Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 220, 50, 50, 200);
    for (int i = 0; i < g->deaths && i < 10; i++) {
        SDL_Rect r = { 10 + i * 18, 10, 14, 14 };
        SDL_RenderFillRect(g->renderer, &r);
    }
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 80);
    SDL_Rect label = { 6, 6, 10 + (g->deaths ? g->deaths : 1) * 18, 22 };
    SDL_RenderDrawRect(g->renderer, &label);
}

void game_render(Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 20, 22, 40, 255);
    SDL_RenderClear(g->renderer);

    /* Subtle two-tone background */
    SDL_SetRenderDrawColor(g->renderer, 30, 32, 55, 255);
    for (int y = WINDOW_H / 2; y < WINDOW_H; y++)
        SDL_RenderDrawLine(g->renderer, 0, y, WINDOW_W, y);

    level_render(&g->level, g->renderer, g->sprites.tile);

    for (int i = 0; i < g->enemy_count; i++)
        enemy_render(&g->enemies[i], g->renderer, g->sprites.enemy);

    player_render(&g->player, g->renderer, g->sprites.player);

    draw_hud(g);
    SDL_RenderPresent(g->renderer);
}

void game_shutdown(Game *g) {
    sprites_free(&g->sprites);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    SDL_Quit();
}
