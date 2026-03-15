#include "game.h"
#include "physics.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

/* Find a free enemy slot, or -1 if none available */
static int find_free_enemy_slot(Game *g) {
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (!g->enemies[i].alive) return i;
    /* Also use beyond current count if room */
    if (g->enemy_count < MAX_ENEMIES) return g->enemy_count;
    return -1;
}

/* Spawn a boss at a random x position near the top of the level */
static void spawn_boss(Game *g) {
    int slot = find_free_enemy_slot(g);
    if (slot < 0) return;

    /* Pick a random column away from the edges */
    int col = 2 + rand() % (g->level.cols - 4);
    float x = (float)(col * TILE_SIZE);
    float y = TILE_SIZE * 2.0f;   /* drop in from near the top */

    enemy_init_boss(&g->enemies[slot], x, y);
    if (slot >= g->enemy_count) g->enemy_count = slot + 1;
}

/* -------------------------------------------------------------------------
   Level loading
   ------------------------------------------------------------------------- */
void game_load_level(Game *g, int level_num) {
    char path[64];
    snprintf(path, sizeof(path), "assets/level%d.txt", level_num);
    if (!level_load(&g->level, path)) {
        fprintf(stderr, "game_load_level: '%s' not found, using built-in\n", path);
        level_init_builtin(&g->level);
    }
    g->current_level = level_num;
    player_init(&g->player,
                g->level.player_spawn.x,
                g->level.player_spawn.y);
    spawn_enemies(g);
    lootbox_manager_init(&g->lootboxes);
    bullet_manager_init(&g->bullets);
    background_load(&g->sprites, g->renderer, level_num);
}

/* -------------------------------------------------------------------------
   State transitions
   ------------------------------------------------------------------------- */
void game_goto_title(Game *g) {
    g->state = STATE_TITLE;
    title_init(&g->title, g->music_on);
    if (!g->music_on) audio_stop_music();
    else              audio_play_music(&g->audio);
}

void game_start_playing(Game *g) {
    g->state         = STATE_PLAYING;
    g->score         = 0;
    g->lives         = STARTING_LIVES;
    g->current_level = 1;
    game_load_level(g, 1);
    if (g->music_on) audio_play_music(&g->audio);
    else             audio_stop_music();
}

/* -------------------------------------------------------------------------
   One-time init
   ------------------------------------------------------------------------- */
int game_init(Game *g) {
    memset(g, 0, sizeof(*g));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    g->window = SDL_CreateWindow(
        "Platformer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if (!g->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return -1;
    }

    g->renderer = SDL_CreateRenderer(
        g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return -1;
    }

    sprites_load(&g->sprites, g->renderer);
    audio_load(&g->audio);
    audio_set_music_volume(80);
    audio_set_sfx_volume(&g->audio, 110);

    /* Open the first available game controller */
    g->controller = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            g->controller = SDL_GameControllerOpen(i);
            if (g->controller) break;
        }
    }

    /* Startup diagnostics */
    {
        char cwd[512];
#if defined(_WIN32)
        if (_getcwd(cwd, sizeof(cwd)))
#else
        if (getcwd(cwd, sizeof(cwd)))
#endif
            fprintf(stderr, "INFO: working dir : %s\n", cwd);
#ifdef SDL2_IMAGE_FOUND
        fprintf(stderr, "INFO: SDL2_image  : compiled IN\n");
#else
        fprintf(stderr, "INFO: SDL2_image  : NOT compiled in\n");
#endif
#ifdef SDL2_MIXER_FOUND
        fprintf(stderr, "INFO: SDL2_mixer  : compiled IN\n");
#else
        fprintf(stderr, "INFO: SDL2_mixer  : NOT compiled in\n");
#endif
        fprintf(stderr,
            "INFO: sprites     : player=%s enemy=%s tile=%s "
            "spring=%s lootbox=%s boss=%s\n",
            g->sprites.player  ? "OK" : "missing",
            g->sprites.enemy   ? "OK" : "missing",
            g->sprites.tile    ? "OK" : "missing",
            g->sprites.spring  ? "OK" : "missing",
            g->sprites.lootbox ? "OK" : "missing",
            g->sprites.boss    ? "OK" : "missing");
        fprintf(stderr,
            "INFO: background  : loaded on first level start\n");
    }

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
        if (e.type == SDL_QUIT) { g->state = STATE_QUIT; return; }

        /* --- Controller hot-plug --- */
        if (e.type == SDL_CONTROLLERDEVICEADDED) {
            if (!g->controller) {
                g->controller = SDL_GameControllerOpen(e.cdevice.which);
                fprintf(stderr, "INFO: controller connected: %s\n",
                        g->controller
                            ? SDL_GameControllerName(g->controller)
                            : "(failed to open)");
            }
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (g->controller &&
                SDL_GameControllerGetJoystick(g->controller) ==
                    SDL_JoystickFromInstanceID(e.cdevice.which)) {
                SDL_GameControllerClose(g->controller);
                g->controller = NULL;
                fprintf(stderr, "INFO: controller disconnected\n");
                /* Try to reopen any remaining controller */
                for (int i = 0; i < SDL_NumJoysticks(); i++) {
                    if (SDL_IsGameController(i)) {
                        g->controller = SDL_GameControllerOpen(i);
                        if (g->controller) break;
                    }
                }
            }
        }

        if (g->state == STATE_TITLE) {
            bool start_game, quit, music_toggled;
            title_handle_event(&g->title, &e, g->controller,
                               &start_game, &quit, &music_toggled);
            if (quit)          { g->state = STATE_QUIT; return; }
            if (music_toggled) {
                g->music_on = g->title.music_on;
                if (g->music_on) audio_play_music(&g->audio);
                else             audio_stop_music();
            }
            if (start_game) game_start_playing(g);
            continue;
        }

        if (g->state == STATE_PLAYING) {
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE) {
                game_goto_title(g);
                return;
            }
            /* Controller Start / Back → return to title */
            if (e.type == SDL_CONTROLLERBUTTONDOWN &&
                (e.cbutton.button == SDL_CONTROLLER_BUTTON_START ||
                 e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)) {
                game_goto_title(g);
                return;
            }
        }
    }

    if (g->state == STATE_PLAYING) {
        const uint8_t *keys = SDL_GetKeyboardState(NULL);

        /* Determine jump intent from keyboard OR controller */
        bool ctrl_jump = false;
        if (g->controller) {
            ctrl_jump =
                SDL_GameControllerGetButton(g->controller,
                    SDL_CONTROLLER_BUTTON_A)       != 0 ||
                SDL_GameControllerGetButton(g->controller,
                    SDL_CONTROLLER_BUTTON_B)       != 0 ||
                SDL_GameControllerGetButton(g->controller,
                    SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
        }
        bool jump_pressed  = (keys[SDL_SCANCODE_SPACE] ||
                              keys[SDL_SCANCODE_W]     ||
                              keys[SDL_SCANCODE_UP]    ||
                              ctrl_jump) != 0;
        bool was_on_ground = g->player.on_ground;
        bool shoot_requested = false;
        player_handle_input(&g->player, keys, g->controller, &shoot_requested);
        if (jump_pressed && was_on_ground)
            audio_play_sfx(&g->audio, SFX_JUMP);

        /* Fire a bullet if requested */
        if (shoot_requested && g->player.alive) {
            /* Spawn bullet from the front-centre of the player */
            float bx = (g->player.facing > 0)
                       ? g->player.x + PLAYER_W        /* right side */
                       : g->player.x - BULLET_W;       /* left side */
            float by = g->player.y + PLAYER_H / 2.0f - BULLET_H / 2.0f;
            bullet_fire(&g->bullets, bx, by, g->player.facing);
        }
    }
}

/* -------------------------------------------------------------------------
   Update
   ------------------------------------------------------------------------- */
void game_update(Game *g, float dt) {
    if (g->state != STATE_PLAYING) return;
    if (dt > 0.05f) dt = 0.05f;

    /* --- Player physics --- */
    bool was_on_spring = g->player.on_spring;
    player_update(&g->player, &g->level, dt);

    /* Spring SFX — fire once on the frame the spring launches the player */
    if (!was_on_spring && g->player.on_spring)
        audio_play_sfx(&g->audio, SFX_SPRING);

    /* --- Bullet physics --- */
    bullet_manager_update(&g->bullets, &g->level, dt);

    /* --- Bullet–enemy collision --- */
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!g->bullets.bullets[b].alive) continue;
        for (int i = 0; i < g->enemy_count; i++) {
            if (!g->enemies[i].alive) continue;
            AABB ebox = enemy_aabb(&g->enemies[i]);
            if (bullet_hits(&g->bullets, b, ebox)) {
                g->enemies[i].hp--;
                audio_play_sfx(&g->audio, SFX_STOMP);
                if (g->enemies[i].hp <= 0) {
                    g->enemies[i].alive = false;
                    g->score += (g->enemies[i].type == ENEMY_BOSS) ? 3 : 1;
                }
                break;   /* bullet consumed, stop checking enemies */
            }
        }
    }

    for (int i = 0; i < g->enemy_count; i++)
        enemy_update(&g->enemies[i], &g->level, dt);

    /* --- Player–enemy collision --- */
    AABB pbox = player_aabb(&g->player);
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].alive) continue;
        AABB ebox = enemy_aabb(&g->enemies[i]);
        if (!aabb_overlap(pbox, ebox)) continue;

        float player_bottom = g->player.y + PLAYER_H;
        float enemy_top     = g->enemies[i].y;

        if (g->player.vy > 0 && player_bottom - enemy_top < 16) {
            /* Stomp — deal one HP of damage */
            g->enemies[i].hp--;
            g->player.vy = PLAYER_JUMP * 0.6f;
            audio_play_sfx(&g->audio, SFX_STOMP);
            if (g->enemies[i].hp <= 0) {
                g->enemies[i].alive = false;
                /* Score: boss worth 3, normal worth 1 */
                g->score += (g->enemies[i].type == ENEMY_BOSS) ? 3 : 1;
            }
        } else {
            /* Hit by enemy */
            g->lives--;
            audio_play_sfx(&g->audio, SFX_DEATH);
            if (g->lives <= 0) {
                game_goto_title(g);
                return;
            }
            player_init(&g->player,
                        g->level.player_spawn.x,
                        g->level.player_spawn.y);
        }
    }

    /* --- Loot boxes: only spawn while normal enemies are still alive --- */
    if (!all_enemies_dead(g)) {
        int spawned = lootbox_manager_update(&g->lootboxes, &g->level, dt);
        if (spawned >= 0)
            audio_play_sfx(&g->audio, SFX_LOOTBOX_SPAWN);
    }

    int opened = lootbox_check_open(&g->lootboxes, pbox);
    if (opened >= 0) {
        g->score += 10;
        audio_play_sfx(&g->audio, SFX_LOOTBOX_OPEN);
        spawn_boss(g);
        audio_play_sfx(&g->audio, SFX_BOSS_SPAWN);
    }

    /* --- Level clear: all enemies dead and no already-active loot boxes --- */
    bool any_lootbox_active = false;
    for (int i = 0; i < MAX_LOOTBOXES; i++)
        if (g->lootboxes.boxes[i].active) { any_lootbox_active = true; break; }

    if (g->enemy_count > 0 && all_enemies_dead(g) && !any_lootbox_active) {
        int next = g->current_level + 1;
        char path[64];
        snprintf(path, sizeof(path), "assets/level%d.txt", next);
        FILE *f = fopen(path, "r");
        if (f) { fclose(f); game_load_level(g, next); }
        else   { game_load_level(g, 1); }
    }
}

/* -------------------------------------------------------------------------
   HUD
   ------------------------------------------------------------------------- */
static void draw_digit(SDL_Renderer *r, int d, int x, int y,
                        int seg_w, int seg_h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    static const uint8_t SEG[10] = {
        0x77,0x24,0x5D,0x6D,0x2E,0x6B,0x7B,0x25,0x7F,0x6F
    };
    uint8_t s = SEG[d];
    int w = seg_w, h = seg_h, t = 3;
    if (s&0x40){SDL_Rect rc={x,     y,     w,t};SDL_RenderFillRect(r,&rc);}
    if (s&0x20){SDL_Rect rc={x,     y,     t,h};SDL_RenderFillRect(r,&rc);}
    if (s&0x10){SDL_Rect rc={x+w-t, y,     t,h};SDL_RenderFillRect(r,&rc);}
    if (s&0x08){SDL_Rect rc={x,     y+h-t, w,t};SDL_RenderFillRect(r,&rc);}
    if (s&0x04){SDL_Rect rc={x,     y+h-t, t,h};SDL_RenderFillRect(r,&rc);}
    if (s&0x02){SDL_Rect rc={x+w-t, y+h-t, t,h};SDL_RenderFillRect(r,&rc);}
    if (s&0x01){SDL_Rect rc={x,     y+h*2-t,w,t};SDL_RenderFillRect(r,&rc);}
}

static void draw_number_right(SDL_Renderer *r, int value, int right_x, int y) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    int seg_w = 16, seg_h = 14, gap = 4;
    int digit_w = seg_w + gap;
    int len = (int)strlen(buf);
    SDL_Color col = {255, 220, 60, 255};
    for (int i = len - 1; i >= 0; i--) {
        int d = buf[i] - '0';
        int x = right_x - (len - i) * digit_w;
        draw_digit(r, d, x, y, seg_w, seg_h, col);
    }
}

static void draw_hud(Game *g) {
    /* Lives — green squares top-left */
    for (int i = 0; i < STARTING_LIVES; i++) {
        SDL_Color c = (i < g->lives)
            ? (SDL_Color){80, 200, 80, 255}
            : (SDL_Color){50, 50,  50, 180};
        SDL_SetRenderDrawColor(g->renderer, c.r, c.g, c.b, c.a);
        SDL_Rect sq = {14 + i * 22, 14, 16, 16};
        SDL_RenderFillRect(g->renderer, &sq);
    }

    /* Score — top right */
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 200);
    SDL_Rect score_bg = {WINDOW_W - 160, 8, 152, 44};
    SDL_RenderFillRect(g->renderer, &score_bg);
    SDL_SetRenderDrawColor(g->renderer, 100, 100, 160, 255);
    SDL_RenderDrawRect(g->renderer, &score_bg);
    draw_number_right(g->renderer, g->score, WINDOW_W - 16, 14);

    /* Level — top centre */
    SDL_SetRenderDrawColor(g->renderer, 60, 60, 100, 160);
    SDL_Rect lvl_bg = {WINDOW_W / 2 - 50, 8, 100, 28};
    SDL_RenderFillRect(g->renderer, &lvl_bg);
    draw_number_right(g->renderer, g->current_level, WINDOW_W / 2 + 40, 12);
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

        /* Background — texture if available, else a two-tone gradient fallback */
        if (g->sprites.bg) {
            SDL_Rect screen = { 0, 0, WINDOW_W, WINDOW_H };
            SDL_RenderCopy(g->renderer, g->sprites.bg, NULL, &screen);
        } else {
            /* Fallback: two-tone solid colour */
            SDL_SetRenderDrawColor(g->renderer, 20, 22, 40, 255);
            SDL_Rect top_half = { 0, 0, WINDOW_W, WINDOW_H / 2 };
            SDL_RenderFillRect(g->renderer, &top_half);
            SDL_SetRenderDrawColor(g->renderer, 30, 32, 55, 255);
            SDL_Rect bot_half = { 0, WINDOW_H / 2, WINDOW_W, WINDOW_H / 2 };
            SDL_RenderFillRect(g->renderer, &bot_half);
        }

        level_render(&g->level, g->renderer,
                     g->sprites.tile, g->sprites.spring);

        lootbox_render(&g->lootboxes, g->renderer, g->sprites.lootbox);

        for (int i = 0; i < g->enemy_count; i++)
            enemy_render(&g->enemies[i], g->renderer,
                         g->sprites.enemy, g->sprites.boss);

        player_render(&g->player, g->renderer, g->sprites.player);

        bullet_manager_render(&g->bullets, g->renderer);

        draw_hud(g);
        SDL_RenderPresent(g->renderer);
    }
}

/* -------------------------------------------------------------------------
   Shutdown
   ------------------------------------------------------------------------- */
void game_shutdown(Game *g) {
    if (g->controller) { SDL_GameControllerClose(g->controller); g->controller = NULL; }
    audio_free(&g->audio);
    sprites_free(&g->sprites);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    SDL_Quit();
}
