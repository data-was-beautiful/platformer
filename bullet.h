#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"
#include "level.h"

#define MAX_BULLETS      16
#define BULLET_W          8
#define BULLET_H          5
#define BULLET_SPEED    480.0f   /* pixels per second */

typedef struct {
    float x, y;
    float vx;        /* horizontal only — positive = right */
    bool  alive;
} Bullet;

typedef struct {
    Bullet bullets[MAX_BULLETS];
} BulletManager;

void   bullet_manager_init(BulletManager *bm);

/* Fire a bullet from (x, y) in direction dir (+1 right, -1 left).
   Silently drops if the pool is full. */
void   bullet_fire(BulletManager *bm, float x, float y, int dir);

/* Move all bullets; kill any that hit a solid tile.
   Returns the index of the first newly-killed bullet this frame, or -1. */
void   bullet_manager_update(BulletManager *bm, const Level *lvl, float dt);

/* Returns true if bullet i overlaps the given AABB, and kills the bullet. */
bool   bullet_hits(BulletManager *bm, int i, AABB target);

AABB   bullet_aabb(const Bullet *b);

void   bullet_manager_render(const BulletManager *bm, SDL_Renderer *renderer);

#endif
