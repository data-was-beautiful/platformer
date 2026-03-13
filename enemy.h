#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "physics.h"
#include "level.h"

#define ENEMY_W         24
#define ENEMY_H         24
#define ENEMY_SPEED     80.0f
#define MAX_ENEMIES     8
#define GRAVITY         900.0f

typedef struct {
    float x, y;
    float vx, vy;
    bool  on_ground;
    bool  alive;
    int   dir;       /* +1 right, -1 left */
} Enemy;

void enemy_init(Enemy *e, float x, float y);
void enemy_update(Enemy *e, const Level *lvl, float dt);
void enemy_render(const Enemy *e, struct SDL_Renderer *renderer);
AABB enemy_aabb(const Enemy *e);

#endif
