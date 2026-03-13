#include "game.h"
#include "physics.h"
#include <stdio.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
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

    /* Load audio (graceful: no-ops if SDL2_mixer absent or files missing) */
    audio_load(&g->audio);
    audio_set_music_volume(80);   /* 0–128; tweak to taste */
    audio_set_sfx_volume(&g->audio, 110);

    /* Startup diagnostics — printed to terminal on launch */
    {
        char cwd[512];
        if (SDL_GetBasePath()) {
            fprintf(stderr, "INFO: SDL base path : %s\n", SDL_GetBasePath());
        }
#if defined(_WIN32)
        if (_getcwd(cwd, sizeof(cwd)))
#else
        if (getcwd(cwd, sizeof(cwd)))
#endif
            fprintf(stderr, "INFO: working dir   : %s\n", cwd);

#ifdef SDL2_IMAGE_FOUND
        fprintf(stderr, "INFO: SDL2_image     : compiled IN\n");
#else
        fprintf(stderr, "INFO: SDL2_image     : NOT compiled in (rect fallback)\n");
#endif
        fprintf(stderr, "INFO: sprites.player : %s\n",
                g->sprites.player ? "LOADED" : "NULL (missing/failed)");
        fprintf(stderr, "INFO: sprites.enemy  : %s\n",
                g->sprites.enemy  ? "LOADED" : "NULL (missing/failed)");
        fprintf(stderr, "INFO: sprites.tile   : %s\n",
                g->sprites.tile   ? "LOADED" : "NULL (missing/failed)");
    }

    /* Spawn player and enemies from level data */
    player_init(&g->player,
                g->level.player_spawn.x,
                g->level.player_spawn.y);
    spawn_enemies(g);

    audio_play_music(&g->audio);

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

    /* Detect jump press this frame so we can fire the SFX exactly once */
    bool jump_pressed = (keys[SDL_SCANCODE_SPACE] ||
                         keys[SDL_SCANCODE_W]     ||
                         keys[SDL_SCANCODE_UP]) != 0;
    bool was_on_ground = g->player.on_ground;

    player_handle_input(&g->player, keys);

    /* Play jump SFX on the frame the player leaves the ground */
    if (jump_pressed && was_on_ground)
        audio_play_sfx(&g->audio, SFX_JUMP);
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
                audio_play_sfx(&g->audio, SFX_STOMP);
            } else {
                g->deaths++;
                audio_play_sfx(&g->audio, SFX_DEATH);
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
    audio_free(&g->audio);
    sprites_free(&g->sprites);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    SDL_Quit();
}
