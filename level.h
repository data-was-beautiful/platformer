#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "physics.h"

#define TILE_SIZE       32
#define LEVEL_COLS_MAX  80
#define LEVEL_ROWS_MAX  40

/* Keep old names as aliases so player.c / enemy.c compile unchanged */
#define LEVEL_COLS  LEVEL_COLS_MAX
#define LEVEL_ROWS  LEVEL_ROWS_MAX

/* Tile types */
#define TILE_AIR    0
#define TILE_SOLID  1

/* Maximum enemy spawns readable from a level file */
#define MAX_SPAWN_ENEMIES  16

typedef struct {
    float x, y;
} SpawnPoint;

typedef struct {
    uint8_t tiles[LEVEL_ROWS_MAX][LEVEL_COLS_MAX];
    int cols;           /* actual columns in this level  */
    int rows;           /* actual rows in this level     */
    int pixel_width;
    int pixel_height;

    /* Spawn points parsed from the level file ('P' and 'E' chars) */
    SpawnPoint player_spawn;
    SpawnPoint enemy_spawns[MAX_SPAWN_ENEMIES];
    int        enemy_spawn_count;
} Level;

/* Load level from a text file.
   Returns true on success, false on failure. */
bool level_load(Level *lvl, const char *path);

/* Initialise with the built-in hardcoded map (always succeeds) */
void level_init_builtin(Level *lvl);

/* Returns true if tile at grid cell (col, row) is solid */
bool level_is_solid(const Level *lvl, int col, int row);

/* Returns true if the AABB overlaps any solid tile.
   If so, out_col/out_row are set to the first offending tile. */
bool level_collides(const Level *lvl, AABB box,
                    int *out_col, int *out_row);

/* Draw all solid tiles — uses tile texture if non-NULL, else filled rect */
void level_render(const Level *lvl, SDL_Renderer *renderer,
                  SDL_Texture *tile_tex);

#endif
