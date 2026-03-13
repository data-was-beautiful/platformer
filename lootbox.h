#ifndef LOOTBOX_H
#define LOOTBOX_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"
#include "level.h"

#define LOOTBOX_W        24
#define LOOTBOX_H        24
#define MAX_LOOTBOXES     4

/* Seconds between random loot box spawns (min and max of random range) */
#define LOOTBOX_SPAWN_MIN  8.0f
#define LOOTBOX_SPAWN_MAX 18.0f

typedef struct {
    float x, y;
    bool  active;
    float bob_t;    /* time accumulator for bobbing animation */
} LootBox;

typedef struct {
    LootBox boxes[MAX_LOOTBOXES];
    float   spawn_timer;    /* counts down to next spawn */
    float   next_spawn;     /* randomised interval       */
} LootBoxManager;

void lootbox_manager_init(LootBoxManager *m);

/* Call every frame. Returns index of newly spawned box (-1 if none). */
int  lootbox_manager_update(LootBoxManager *m, const Level *lvl, float dt);

/* Check player AABB against all active boxes.
   Returns index of opened box (-1 if none), deactivates it. */
int  lootbox_check_open(LootBoxManager *m, AABB player_box);

void lootbox_render(const LootBoxManager *m, SDL_Renderer *renderer,
                    SDL_Texture *tex);

AABB lootbox_aabb(const LootBox *b);

#endif
