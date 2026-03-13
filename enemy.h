#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"
#include "level.h"

/* Normal enemy dimensions */
#define ENEMY_W         24
#define ENEMY_H         24
#define ENEMY_SPEED     80.0f

/* Boss enemy dimensions */
#define BOSS_W          40
#define BOSS_H          40
#define BOSS_SPEED      50.0f
#define BOSS_MAX_HP      3

/* Pool size — normal + boss enemies share one array */
#define MAX_ENEMIES     16

#define GRAVITY         900.0f

typedef enum {
    ENEMY_NORMAL = 0,
    ENEMY_BOSS   = 1
} EnemyType;

typedef struct {
    float     x, y;
    float     vx, vy;
    bool      on_ground;
    bool      alive;
    int       dir;       /* +1 right, -1 left */
    EnemyType type;
    int       hp;        /* normal=1, boss=3    */
} Enemy;

/* Initialise as a normal patrol enemy */
void enemy_init(Enemy *e, float x, float y);

/* Initialise as a boss enemy */
void enemy_init_boss(Enemy *e, float x, float y);

void enemy_update(Enemy *e, const Level *lvl, float dt);

/* Returns the AABB for this enemy (size depends on type) */
AABB enemy_aabb(const Enemy *e);

/* tex = normal enemy texture, boss_tex = boss texture.
   Pass NULL for coloured-rect fallback. */
void enemy_render(const Enemy *e, SDL_Renderer *renderer,
                  SDL_Texture *tex, SDL_Texture *boss_tex);

#endif
