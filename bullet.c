#include "bullet.h"
#include <string.h>

AABB bullet_aabb(const Bullet *b) {
    return (AABB){ b->x, b->y, BULLET_W, BULLET_H };
}

void bullet_manager_init(BulletManager *bm) {
    memset(bm, 0, sizeof(*bm));
}

void bullet_fire(BulletManager *bm, float x, float y, int dir) {
    /* Find a free slot */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bm->bullets[i].alive) {
            bm->bullets[i].x     = x;
            bm->bullets[i].y     = y;
            bm->bullets[i].vx    = BULLET_SPEED * dir;
            bm->bullets[i].alive = true;
            return;
        }
    }
    /* Pool full — silently drop */
}

void bullet_manager_update(BulletManager *bm, const Level *lvl, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *b = &bm->bullets[i];
        if (!b->alive) continue;

        b->x += b->vx * dt;

        /* Kill if it flies off the level bounds */
        if (b->x + BULLET_W < 0 || b->x > lvl->pixel_width) {
            b->alive = false;
            continue;
        }

        /* Kill on tile collision */
        int tc, tr;
        if (level_collides(lvl, bullet_aabb(b), &tc, &tr)) {
            b->alive = false;
        }
    }
}

bool bullet_hits(BulletManager *bm, int i, AABB target) {
    Bullet *b = &bm->bullets[i];
    if (!b->alive) return false;
    if (aabb_overlap(bullet_aabb(b), target)) {
        b->alive = false;
        return true;
    }
    return false;
}

void bullet_manager_render(const BulletManager *bm, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 230, 60, 255);   /* bright yellow */
    for (int i = 0; i < MAX_BULLETS; i++) {
        const Bullet *b = &bm->bullets[i];
        if (!b->alive) continue;
        SDL_Rect dst = { (int)b->x, (int)b->y, BULLET_W, BULLET_H };
        SDL_RenderFillRect(renderer, &dst);

        /* Small bright core */
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
        SDL_Rect core = { (int)b->x + 2, (int)b->y + 1, BULLET_W - 4, BULLET_H - 2 };
        SDL_RenderFillRect(renderer, &core);
        SDL_SetRenderDrawColor(renderer, 255, 230, 60, 255);
    }
}
