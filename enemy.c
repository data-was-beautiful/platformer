#include "enemy.h"
#include <SDL2/SDL.h>

AABB enemy_aabb(const Enemy *e) {
    int w = (e->type == ENEMY_BOSS) ? BOSS_W : ENEMY_W;
    int h = (e->type == ENEMY_BOSS) ? BOSS_H : ENEMY_H;
    return (AABB){ e->x, e->y, (float)w, (float)h };
}

void enemy_init(Enemy *e, float x, float y) {
    e->x        = x;
    e->y        = y;
    e->vx       = ENEMY_SPEED;
    e->vy       = 0;
    e->on_ground= false;
    e->alive    = true;
    e->dir      = 1;
    e->type     = ENEMY_NORMAL;
    e->hp       = 1;
}

void enemy_init_boss(Enemy *e, float x, float y) {
    e->x        = x;
    e->y        = y;
    e->vx       = BOSS_SPEED;
    e->vy       = 0;
    e->on_ground= false;
    e->alive    = true;
    e->dir      = 1;
    e->type     = ENEMY_BOSS;
    e->hp       = BOSS_MAX_HP;
}

static void flip(Enemy *e) {
    e->dir = -e->dir;
    float spd = (e->type == ENEMY_BOSS) ? BOSS_SPEED : ENEMY_SPEED;
    e->vx = spd * e->dir;
}

void enemy_update(Enemy *e, const Level *lvl, float dt) {
    if (!e->alive) return;

    int ew = (e->type == ENEMY_BOSS) ? BOSS_W : ENEMY_W;
    int eh = (e->type == ENEMY_BOSS) ? BOSS_H : ENEMY_H;

    e->vy += GRAVITY * dt;

    /* --- X axis --- */
    e->x += e->vx * dt;
    {
        int tc, tr;
        AABB box = enemy_aabb(e);
        if (level_collides(lvl, box, &tc, &tr)) {
            if (e->dir > 0)
                e->x = (float)(tc * TILE_SIZE) - ew;
            else
                e->x = (float)(tc * TILE_SIZE) + TILE_SIZE;
            flip(e);
        }
    }

    /* Ledge detection */
    {
        float probe_x = (e->dir > 0) ? (e->x + ew + 1) : (e->x - 1);
        float probe_y = e->y + eh + 1;
        int check_col = (int)probe_x / TILE_SIZE;
        int check_row = (int)probe_y / TILE_SIZE;
        if (e->on_ground && !level_is_solid(lvl, check_col, check_row))
            flip(e);
    }

    if (e->x < 0 || e->x + ew > lvl->pixel_width) flip(e);

    /* --- Y axis --- */
    e->on_ground = false;
    e->y += e->vy * dt;
    {
        int tc, tr;
        AABB box = enemy_aabb(e);
        if (level_collides(lvl, box, &tc, &tr)) {
            float tile_top    = (float)(tr * TILE_SIZE);
            float tile_bottom = tile_top + TILE_SIZE;
            if (e->vy > 0) {
                e->y = tile_top - eh;
                e->on_ground = true;
            } else {
                e->y = tile_bottom;
            }
            e->vy = 0;
        }

        /* Spring launch */
        AABB box2 = enemy_aabb(e);
        if (level_on_spring(lvl, box2, NULL, NULL)) {
            e->vy = -650.0f;
            e->on_ground = false;
        }
    }

    if (e->y > lvl->pixel_height + 100) e->alive = false;
}

void enemy_render(const Enemy *e, SDL_Renderer *renderer,
                  SDL_Texture *tex, SDL_Texture *boss_tex) {
    if (!e->alive) return;

    int ew = (e->type == ENEMY_BOSS) ? BOSS_W : ENEMY_W;
    int eh = (e->type == ENEMY_BOSS) ? BOSS_H : ENEMY_H;
    SDL_Rect dst = { (int)e->x, (int)e->y, ew, eh };

    SDL_RendererFlip hflip = (e->dir < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (e->type == ENEMY_BOSS) {
        if (boss_tex) {
            SDL_RenderCopyEx(renderer, boss_tex, NULL, &dst, 0.0, NULL, hflip);
        } else {
            /* Fallback: dark purple rect */
            SDL_SetRenderDrawColor(renderer, 130, 30, 180, 255);
            SDL_RenderFillRect(renderer, &dst);
            SDL_SetRenderDrawColor(renderer, 80, 10, 120, 255);
            SDL_RenderDrawRect(renderer, &dst);
            /* Eye */
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            int ex = (e->dir > 0) ? (int)e->x + 24 : (int)e->x + 6;
            SDL_Rect eye = { ex, (int)e->y + 8, 8, 8 };
            SDL_RenderFillRect(renderer, &eye);
        }

        /* HP bar above boss */
        int bar_w = ew;
        int bar_h = 5;
        int bar_x = (int)e->x;
        int bar_y = (int)e->y - 10;
        SDL_SetRenderDrawColor(renderer, 60, 0, 0, 200);
        SDL_Rect bar_bg = { bar_x, bar_y, bar_w, bar_h };
        SDL_RenderFillRect(renderer, &bar_bg);
        int filled = (bar_w * e->hp) / BOSS_MAX_HP;
        SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
        SDL_Rect bar_fill = { bar_x, bar_y, filled, bar_h };
        SDL_RenderFillRect(renderer, &bar_fill);

    } else {
        if (tex) {
            SDL_RenderCopyEx(renderer, tex, NULL, &dst, 0.0, NULL, hflip);
        } else {
            SDL_SetRenderDrawColor(renderer, 210, 60, 60, 255);
            SDL_RenderFillRect(renderer, &dst);
            SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
            int eye_ox = (e->dir > 0) ? 14 : 2;
            SDL_Rect eye = { (int)e->x + eye_ox, (int)e->y + 6, 5, 5 };
            SDL_RenderFillRect(renderer, &eye);
            SDL_SetRenderDrawColor(renderer, 140, 20, 20, 255);
            SDL_RenderDrawRect(renderer, &dst);
        }
    }
}
