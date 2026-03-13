#include "lootbox.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>

AABB lootbox_aabb(const LootBox *b) {
    return (AABB){ b->x, b->y, LOOTBOX_W, LOOTBOX_H };
}

/* Pick a random float in [lo, hi] */
static float randf(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

void lootbox_manager_init(LootBoxManager *m) {
    for (int i = 0; i < MAX_LOOTBOXES; i++)
        m->boxes[i].active = false;
    m->spawn_timer = randf(LOOTBOX_SPAWN_MIN, LOOTBOX_SPAWN_MAX);
    m->next_spawn  = m->spawn_timer;
}

/* Find a free slot, or -1 if all occupied */
static int find_free_slot(LootBoxManager *m) {
    for (int i = 0; i < MAX_LOOTBOXES; i++)
        if (!m->boxes[i].active) return i;
    return -1;
}

/* Pick a spawn position: hover one tile above a random solid tile */
typedef struct { int c, r; } Candidate;

static bool pick_spawn(const Level *lvl, float *out_x, float *out_y) {
    /* Collect candidate tiles that have air above them */
    static Candidate candidates[LEVEL_COLS_MAX * LEVEL_ROWS_MAX];
    int count = 0;
    int max_cands = LEVEL_COLS_MAX * LEVEL_ROWS_MAX;

    for (int r = 1; r < lvl->rows - 1 && count < max_cands; r++) {
        for (int c = 1; c < lvl->cols - 1 && count < max_cands; c++) {
            uint8_t t      = level_tile_at(lvl, c, r);
            uint8_t above  = level_tile_at(lvl, c, r - 1);
            uint8_t above2 = level_tile_at(lvl, c, r - 2);
            if ((t == TILE_SOLID || t == TILE_SPRING) &&
                above == TILE_AIR && above2 == TILE_AIR) {
                candidates[count].c = c;
                candidates[count].r = r;
                count++;
            }
        }
    }

    if (count == 0) return false;

    int idx = rand() % count;
    *out_x = (float)(candidates[idx].c * TILE_SIZE) +
             (TILE_SIZE - LOOTBOX_W) / 2.0f;
    *out_y = (float)((candidates[idx].r - 2) * TILE_SIZE);
    return true;
}

int lootbox_manager_update(LootBoxManager *m, const Level *lvl, float dt) {
    /* Update bobbing animation */
    for (int i = 0; i < MAX_LOOTBOXES; i++)
        if (m->boxes[i].active)
            m->boxes[i].bob_t += dt;

    /* Countdown to next spawn */
    m->spawn_timer -= dt;
    if (m->spawn_timer > 0.0f) return -1;

    /* Reset timer for next spawn */
    m->spawn_timer = randf(LOOTBOX_SPAWN_MIN, LOOTBOX_SPAWN_MAX);

    int slot = find_free_slot(m);
    if (slot < 0) return -1;   /* all slots in use */

    float sx, sy;
    if (!pick_spawn(lvl, &sx, &sy)) return -1;

    m->boxes[slot].x      = sx;
    m->boxes[slot].y      = sy;
    m->boxes[slot].active = true;
    m->boxes[slot].bob_t  = 0.0f;
    return slot;
}

int lootbox_check_open(LootBoxManager *m, AABB player_box) {
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        if (!m->boxes[i].active) continue;
        AABB bx = lootbox_aabb(&m->boxes[i]);
        /* Simple overlap check */
        if (player_box.x < bx.x + bx.w &&
            player_box.x + player_box.w > bx.x &&
            player_box.y < bx.y + bx.h &&
            player_box.y + player_box.h > bx.y) {
            m->boxes[i].active = false;
            return i;
        }
    }
    return -1;
}

void lootbox_render(const LootBoxManager *m, SDL_Renderer *renderer,
                    SDL_Texture *tex) {
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        if (!m->boxes[i].active) continue;

        /* Gentle bob: ±4 pixels using a sine wave */
        float bob = sinf(m->boxes[i].bob_t * 3.0f) * 4.0f;
        SDL_Rect dst = {
            (int)m->boxes[i].x,
            (int)(m->boxes[i].y + bob),
            LOOTBOX_W,
            LOOTBOX_H
        };

        if (tex) {
            SDL_RenderCopy(renderer, tex, NULL, &dst);
        } else {
            /* Fallback: golden box with question-mark style cross */
            SDL_SetRenderDrawColor(renderer, 240, 190, 30, 255);
            SDL_RenderFillRect(renderer, &dst);
            SDL_SetRenderDrawColor(renderer, 180, 120, 10, 255);
            SDL_RenderDrawRect(renderer, &dst);
            /* Inner cross to suggest "?" */
            SDL_SetRenderDrawColor(renderer, 200, 140, 20, 255);
            SDL_RenderDrawLine(renderer,
                dst.x + dst.w/2, dst.y + 4,
                dst.x + dst.w/2, dst.y + dst.h - 4);
            SDL_RenderDrawLine(renderer,
                dst.x + 4,       dst.y + dst.h/2,
                dst.x + dst.w-4, dst.y + dst.h/2);

            /* Shimmer dots */
            SDL_SetRenderDrawColor(renderer, 255, 240, 120, 200);
            SDL_Rect shine = { dst.x + 3, dst.y + 3, 4, 4 };
            SDL_RenderFillRect(renderer, &shine);
        }
    }
}
