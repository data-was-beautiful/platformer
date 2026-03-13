#include "enemy.h"
#include <SDL2/SDL.h>

AABB enemy_aabb(const Enemy *e) {
    return (AABB){ e->x, e->y, ENEMY_W, ENEMY_H };
}

void enemy_init(Enemy *e, float x, float y) {
    e->x = x;
    e->y = y;
    e->vx = ENEMY_SPEED;
    e->vy = 0;
    e->on_ground = false;
    e->alive = true;
    e->dir = 1;
}

static void flip(Enemy *e) {
    e->dir = -e->dir;
    e->vx  = ENEMY_SPEED * e->dir;
}

void enemy_update(Enemy *e, const Level *lvl, float dt) {
    if (!e->alive) return;

    /* gravity */
    e->vy += GRAVITY * dt;

    /* --- X axis --- */
    e->x += e->vx * dt;
    {
        int tc, tr;
        AABB box = enemy_aabb(e);
        if (level_collides(lvl, box, &tc, &tr)) {
            if (e->dir > 0)
                e->x = (float)(tc * TILE_SIZE) - ENEMY_W;
            else
                e->x = (float)(tc * TILE_SIZE) + TILE_SIZE;
            flip(e);
        }
    }

    /* ledge detection — check one tile ahead and one below */
    {
        int check_col, check_row;
        float probe_x = (e->dir > 0) ? (e->x + ENEMY_W + 1) : (e->x - 1);
        float probe_y = e->y + ENEMY_H + 1; /* just below the enemy */
        check_col = (int)probe_x / TILE_SIZE;
        check_row = (int)probe_y / TILE_SIZE;
        if (e->on_ground && !level_is_solid(lvl, check_col, check_row))
            flip(e);
    }

    /* level horizontal clamp */
    if (e->x < 0 || e->x + ENEMY_W > lvl->pixel_width) flip(e);

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
                e->y = tile_top - ENEMY_H;
                e->on_ground = true;
            } else {
                e->y = tile_bottom;
            }
            e->vy = 0;
        }
    }

    /* fell off bottom */
    if (e->y > lvl->pixel_height + 100) e->alive = false;
}

void enemy_render(const Enemy *e, SDL_Renderer *renderer,
                  SDL_Texture *tex) {
    if (!e->alive) return;

    SDL_Rect dst = { (int)e->x, (int)e->y, ENEMY_W, ENEMY_H };

    if (tex) {
        SDL_RendererFlip flip = (e->dir < 0) ? SDL_FLIP_HORIZONTAL
                                              : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, tex, NULL, &dst, 0.0, NULL, flip);
    } else {
        /* Fallback: coloured rectangles */
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
