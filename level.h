#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"

#define TILE_SIZE       32
#define LEVEL_COLS_MAX  80
#define LEVEL_ROWS_MAX  40

/* Keep old names as aliases */
#define LEVEL_COLS  LEVEL_COLS_MAX
#define LEVEL_ROWS  LEVEL_ROWS_MAX

/* Tile types */
#define TILE_AIR    0
#define TILE_SOLID  1
#define TILE_SPRING 2   /* 'S' in level files — launches entities upward */

/* Maximum enemy spawns readable from a level file */
#define MAX_SPAWN_ENEMIES  16

typedef struct {
    float x, y;
} SpawnPoint;

typedef struct {
    uint8_t tiles[LEVEL_ROWS_MAX][LEVEL_COLS_MAX];
    int cols;
    int rows;
    int pixel_width;
    int pixel_height;

    SpawnPoint player_spawn;
    SpawnPoint enemy_spawns[MAX_SPAWN_ENEMIES];
    int        enemy_spawn_count;
} Level;

bool level_load(Level *lvl, const char *path);
void level_init_builtin(Level *lvl);

/* Returns the tile type at (col, row), TILE_SOLID for out-of-bounds */
uint8_t level_tile_at(const Level *lvl, int col, int row);

bool level_is_solid(const Level *lvl, int col, int row);

/* Collision against SOLID tiles only (spring tiles are NOT solid —
   they are handled separately in physics so entities pass through
   from below but land on top). */
bool level_collides(const Level *lvl, AABB box,
                    int *out_col, int *out_row);

/* Check if entity is resting on a spring tile this frame.
   Call after Y-axis movement resolves. Returns true and sets
   out_col/out_row if the tile directly below the entity is TILE_SPRING. */
bool level_on_spring(const Level *lvl, AABB box,
                     int *out_col, int *out_row);

/* Draw solid and spring tiles */
void level_render(const Level *lvl, SDL_Renderer *renderer,
                  SDL_Texture *tile_tex, SDL_Texture *spring_tex);

#endif
