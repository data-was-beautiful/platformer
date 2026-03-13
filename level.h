#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
#include <stdbool.h>
#include "physics.h"

#define TILE_SIZE   32
#define LEVEL_COLS  40
#define LEVEL_ROWS  20

/* Tile types */
#define TILE_AIR    0
#define TILE_SOLID  1

typedef struct {
    uint8_t tiles[LEVEL_ROWS][LEVEL_COLS];
    int pixel_width;   /* LEVEL_COLS * TILE_SIZE */
    int pixel_height;  /* LEVEL_ROWS * TILE_SIZE */
} Level;

/* Initialise the hardcoded level */
void level_init(Level *lvl);

/* Returns true if tile at grid cell (col, row) is solid */
bool level_is_solid(const Level *lvl, int col, int row);

/* Returns true if the AABB overlaps any solid tile.
   If so, out_col/out_row are set to the first offending tile. */
bool level_collides(const Level *lvl, AABB box,
                    int *out_col, int *out_row);

/* Draw all solid tiles as grey rectangles */
void level_render(const Level *lvl, struct SDL_Renderer *renderer);

#endif
