#include "game.h"
#include "physics.h"
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

/* -------------------------------------------------------------------------
   Internal helpers
   ------------------------------------------------------------------------- */
static void spawn_enemies(Game *g) {
    g->enemy_count = g->level.enemy_spawn_count;
    if (g->enemy_count > MAX_ENEMIES) g->enemy_count = MAX_ENEMIES;
    for (int i = 0; i < g->enemy_count; i++)
        enemy_init(&g->enemies[i],
                   g->level.enemy_spawns[i].x,
                   g->level.enemy_spawns[i].y);
}

static int all_enemies_dead(Game *g) {
    for (int i = 0; i < g->enemy_count; i++)
        if (g->enemies[i].alive) return 0;
    return 1;
}

/* -------------------------------------------------------------------------
   Level loading
   ------------------------------------------------------------------------- */
void game_load_level(Game *g, int level_num) {
    char path[64];
    snprintf(path, sizeof(path), "assets/level%d.txt", level_num);
    if (!level_load(&g->level, path)) {
        fprintf(stderr, "game_load_level: '%s' not found, using built-in\n",
                path);
        level_init_builtin(&g->level);
    }
    g->current_level = level_num;
    player_init(&g->player,
                g->level.player_spawn.x,
                g->level.player_spawn.y);
    spawn_enemies(g);
}

/* -------------------------------------------------------------------------
   State transitions
   ------------------------------------------------------------------------- */
void game_goto_title(Game *g) {
    g->state = STATE_TITLE;
    title_init(&g->title, g->music_on);
    if (!g->music_on)
        audio_stop_music();
    else
        audio_play_music(&g->audio);
}

void game_start_playing(Game *g) {
    g->state         = STATE_PLAYING;
    g->score         = 0;
    g->lives         = STARTING_LIVES;
    g->current_level = 1;
    game_load_level(g, 1);
    if (g->music_on)
        audio_play_music(&g->audio);
    else
        audio_stop_music();
}

/* -------------------------------------------------------------------------
   One-time init
   ------------------------------------------------------------------------- */
int game_init(Game *g) {
    memset(g, 0, sizeof(*g));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    g->window = SDL_CreateWindow(
        "Platformer",
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

    sprites_load(&g->sprites, g->renderer);
    audio_load(&g->audio);
    audio_set_music_volume(80);
    audio_set_sfx_volume(&g->audio, 110);

    /* Diagnostics */
    {
        char cwd[512];
#if defined(_WIN32)
        if (_getcwd(cwd, sizeof(cwd)))
#else
        if (getcwd(cwd, sizeof(cwd)))
#endif
            fprintf(stderr, "INFO: working dir  : %s\n", cwd);
#ifdef SDL2_IMAGE_FOUND
        fprintf(stderr, "INFO: SDL2_image   : compiled IN\n");
#else
        fprintf(stderr, "INFO: SDL2_image   : NOT compiled in\n");
#endif
#ifdef SDL2_MIXER_FOUND
        fprintf(stderr, "INFO: SDL2_mixer   : compiled IN\n");
#else
        fprintf(stderr, "INFO: SDL2_mixer   : NOT compiled in\n");
#endif
        fprintf(stderr, "INFO: sprites      : player=%s enemy=%s tile=%s\n",
                g->sprites.player ? "OK" : "missing",
                g->sprites.enemy  ? "OK" : "missing",
                g->sprites.tile   ? "OK" : "missing");
    }

    /* Start on the title screen */
    g->music_on = true;
    game_goto_title(g);

    return 0;
}

/* -------------------------------------------------------------------------
   Event handling
   ------------------------------------------------------------------------- */
void game_handle_events(Game *g) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {

        if (e.type == SDL_QUIT) {
            g->state = STATE_QUIT;
            return;
        }

        if (g->state == STATE_TITLE) {
            bool start_game, quit, music_toggled;
            title_handle_event(&g->title, &e,
                               &start_game, &quit, &music_toggled);
            if (quit) {
                g->state = STATE_QUIT;
                return;
            }
            if (music_toggled) {
                g->music_on = g->title.music_on;
                if (g->music_on)
                    audio_play_music(&g->audio);
                else
                    audio_stop_music();
            }
            if (start_game)
                game_start_playing(g);
            continue;
        }

        if (g->state == STATE_PLAYING) {
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE) {
                game_goto_title(g);
                return;
            }
        }
    }

    /* Per-frame key state for player input (only in PLAYING) */
    if (g->state == STATE_PLAYING) {
        const uint8_t *keys = SDL_GetKeyboardState(NULL);

        bool jump_pressed = (keys[SDL_SCANCODE_SPACE] ||
                             keys[SDL_SCANCODE_W]     ||
                             keys[SDL_SCANCODE_UP]) != 0;
        bool was_on_ground = g->player.on_ground;

        player_handle_input(&g->player, keys);

        if (jump_pressed && was_on_ground)
            audio_play_sfx(&g->audio, SFX_JUMP);
    }
}

/* -------------------------------------------------------------------------
   Update
   ------------------------------------------------------------------------- */
void game_update(Game *g, float dt) {
    if (g->state != STATE_PLAYING) return;
    if (dt > 0.05f) dt = 0.05f;

    player_update(&g->player, &g->level, dt);

    for (int i = 0; i < g->enemy_count; i++)
        enemy_update(&g->enemies[i], &g->level, dt);

    /* Player–enemy collision */
    AABB pbox = player_aabb(&g->player);
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        AABB ebox = enemy_aabb(&g->enemies[i]);
        if (!aabb_overlap(pbox, ebox)) continue;

        float player_bottom = g->player.y + PLAYER_H;
        float enemy_top     = g->enemies[i].y;
        if (g->player.vy > 0 && player_bottom - enemy_top < 16) {
            /* Stomp */
            g->enemies[i].alive = false;
            g->player.vy = PLAYER_JUMP * 0.6f;
            g->score++;
            audio_play_sfx(&g->audio, SFX_STOMP);
        } else {
            /* Hit */
            g->lives--;
            audio_play_sfx(&g->audio, SFX_DEATH);
            if (g->lives <= 0) {
                /* Game over → title */
                game_goto_title(g);
                return;
            }
            /* Respawn on same level */
            player_init(&g->player,
                        g->level.player_spawn.x,
                        g->level.player_spawn.y);
        }
    }

    /* Level clear — all enemies dead */
    if (g->enemy_count > 0 && all_enemies_dead(g)) {
        int next = g->current_level + 1;
        char path[64];
        snprintf(path, sizeof(path), "assets/level%d.txt", next);

        /* Check if next level file exists */
        FILE *f = fopen(path, "r");
        if (f) {
            fclose(f);
            game_load_level(g, next);
        } else {
            /* No more levels — loop back to level 1, keep score */
            game_load_level(g, 1);
        }
    }
}

/* -------------------------------------------------------------------------
   HUD rendering (score top-right, lives top-left)
   ------------------------------------------------------------------------- */

/* Minimal inline bitmap font for HUD numbers/letters
   Uses the same glyph engine as title.c but we call SDL directly here
   to avoid a cross-module dependency on title.c internals.
   We expose a small helper via a shared draw call instead.
   Rather than duplicate the glyph table, we draw the HUD as simple
   coloured indicator blocks — lives as green squares, score as a
   right-aligned number built from SDL_RenderFillRect segments.           */

/* Draw a single decimal digit d (0–9) at (x,y) as a 7-segment-style rect */
static void draw_digit(SDL_Renderer *r, int d, int x, int y,
                        int seg_w, int seg_h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    /* Encode which of 7 segments are on for each digit:
       segments: top, top-left, top-right, middle, bot-left, bot-right, bottom */
    static const uint8_t SEG[10] = {
        0x77, /* 0: all except middle  */
        0x24, /* 1: top-right, bot-right */
        0x5D, /* 2 */
        0x6D, /* 3 */
        0x2E, /* 4 */
        0x6B, /* 5 */
        0x7B, /* 6 */
        0x25, /* 7 */
        0x7F, /* 8 */
        0x6F, /* 9 */
    };
    uint8_t s = SEG[d];
    int w = seg_w, h = seg_h, t = 3; /* thickness */
    /* top */
    if (s & 0x40) { SDL_Rect rc={x,       y,       w, t}; SDL_RenderFillRect(r,&rc); }
    /* top-left */
    if (s & 0x20) { SDL_Rect rc={x,       y,       t, h}; SDL_RenderFillRect(r,&rc); }
    /* top-right */
    if (s & 0x10) { SDL_Rect rc={x+w-t,   y,       t, h}; SDL_RenderFillRect(r,&rc); }
    /* middle */
    if (s & 0x08) { SDL_Rect rc={x,       y+h-t,   w, t}; SDL_RenderFillRect(r,&rc); }
    /* bot-left */
    if (s & 0x04) { SDL_Rect rc={x,       y+h-t,   t, h}; SDL_RenderFillRect(r,&rc); }
    /* bot-right */
    if (s & 0x02) { SDL_Rect rc={x+w-t,   y+h-t,   t, h}; SDL_RenderFillRect(r,&rc); }
    /* bottom */
    if (s & 0x01) { SDL_Rect rc={x,       y+h*2-t, w, t}; SDL_RenderFillRect(r,&rc); }
}

static void draw_number_right(SDL_Renderer *r, int value,
                               int right_x, int y) {
    /* Draw up to 6 digits right-aligned at right_x */
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    int seg_w = 16, seg_h = 14, gap = 4;
    int digit_w = seg_w + gap;
    int len = (int)strlen(buf);
    SDL_Color col = { 255, 220, 60, 255 };
    for (int i = len - 1; i >= 0; i--) {
        int d = buf[i] - '0';
        int x = right_x - (len - i) * digit_w;
        draw_digit(r, d, x, y, seg_w, seg_h, col);
    }
}

static void draw_hud(Game *g) {
    /* Lives — green squares top-left */
    SDL_Color live_col  = { 80,  200, 80,  255 };
    SDL_Color dead_col  = { 50,  50,  50,  180 };
    for (int i = 0; i < STARTING_LIVES; i++) {
        SDL_Color c = (i < g->lives) ? live_col : dead_col;
        SDL_SetRenderDrawColor(g->renderer, c.r, c.g, c.b, c.a);
        SDL_Rect sq = { 14 + i * 22, 14, 16, 16 };
        SDL_RenderFillRect(g->renderer, &sq);
    }

    /* Score label + number — top right */
    /* "SCORE" label as a coloured bar */
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 200);
    SDL_Rect score_bg = { WINDOW_W - 160, 8, 152, 44 };
    SDL_RenderFillRect(g->renderer, &score_bg);
    SDL_SetRenderDrawColor(g->renderer, 100, 100, 160, 255);
    SDL_RenderDrawRect(g->renderer, &score_bg);

    draw_number_right(g->renderer, g->score, WINDOW_W - 16, 14);

    /* Level indicator */
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 160);
    SDL_Rect lvl_bg = { WINDOW_W / 2 - 50, 8, 100, 28 };
    SDL_RenderFillRect(g->renderer, &lvl_bg);
    /* Draw level number small, centred */
    draw_number_right(g->renderer, g->current_level,
                      WINDOW_W / 2 + 40, 12);
}

/* -------------------------------------------------------------------------
   Render
   ------------------------------------------------------------------------- */
void game_render(Game *g) {
    if (g->state == STATE_TITLE) {
        title_render(&g->title, g->renderer, WINDOW_W, WINDOW_H);
        SDL_RenderPresent(g->renderer);
        return;
    }

    if (g->state == STATE_PLAYING) {
        SDL_SetRenderDrawColor(g->renderer, 20, 22, 40, 255);
        SDL_RenderClear(g->renderer);

        SDL_SetRenderDrawColor(g->renderer, 30, 32, 55, 255);
        for (int y = WINDOW_H / 2; y < WINDOW_H; y++)
            SDL_RenderDrawLine(g->renderer, 0, y, WINDOW_W, y);

        level_render(&g->level, g->renderer, g->sprites.tile);

        for (int i = 0; i < g->enemy_count; i++)
            enemy_render(&g->enemies[i], g->renderer, g->sprites.enemy);

        player_render(&g->player, g->renderer, g->sprites.player);

        draw_hud(g);
        SDL_RenderPresent(g->renderer);
        return;
    }
}

/* -------------------------------------------------------------------------
   Shutdown
   ------------------------------------------------------------------------- */
void game_shutdown(Game *g) {
    audio_free(&g->audio);
    sprites_free(&g->sprites);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    SDL_Quit();
}
