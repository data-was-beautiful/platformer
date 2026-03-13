#include "level.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

static const char *BUILTIN_MAP[] = {
    "........................................",
    "........................................",
    "........................................",
    "........................................",
    "........................................",
    "........########........................",
    "........................................",
    ".................#####..................",
    "........................................",
    ".....######.............................",
    "..........................######........",
    "........................................",
    "..............########..................",
    "........................................",
    "........................................",
    "........................................",
    "........................................",
    "........................................",
    "........................................",
    "########################################",
};

static const float DEFAULT_PLAYER_X = 64.0f, DEFAULT_PLAYER_Y = 500.0f;
static const float DEFAULT_ENEMY_X[] = {300,500,700,900,1100,400,600,850};
static const float DEFAULT_ENEMY_Y[] = {550,550,550,550, 550,270,380,290};
#define N_DEFAULT_ENEMIES 8

static void parse_map_chars(Level *lvl, int rows, int cols,
                             const char *lines[]) {
    lvl->rows = rows;
    lvl->cols = cols;
    lvl->enemy_spawn_count = 0;
    lvl->player_spawn.x = DEFAULT_PLAYER_X;
    lvl->player_spawn.y = DEFAULT_PLAYER_Y;

    memset(lvl->tiles, TILE_AIR, sizeof(lvl->tiles));

    for (int r = 0; r < rows; r++) {
        int len = (int)strlen(lines[r]);
        for (int c = 0; c < cols && c < len; c++) {
            char ch = lines[r][c];
            switch (ch) {
            case '#': lvl->tiles[r][c] = TILE_SOLID;  break;
            case 'S': lvl->tiles[r][c] = TILE_SPRING; break;
            case 'P':
                lvl->player_spawn.x = (float)(c * TILE_SIZE);
                lvl->player_spawn.y = (float)(r * TILE_SIZE);
                break;
            case 'E':
                if (lvl->enemy_spawn_count < MAX_SPAWN_ENEMIES) {
                    lvl->enemy_spawns[lvl->enemy_spawn_count].x =
                        (float)(c * TILE_SIZE);
                    lvl->enemy_spawns[lvl->enemy_spawn_count].y =
                        (float)(r * TILE_SIZE);
                    lvl->enemy_spawn_count++;
                }
                break;
            default: break;
            }
        }
    }

    lvl->pixel_width  = cols * TILE_SIZE;
    lvl->pixel_height = rows * TILE_SIZE;
}

void level_init_builtin(Level *lvl) {
    int rows = (int)(sizeof(BUILTIN_MAP) / sizeof(BUILTIN_MAP[0]));
    int cols = (int)strlen(BUILTIN_MAP[0]);
    parse_map_chars(lvl, rows, cols, BUILTIN_MAP);
    lvl->player_spawn.x = DEFAULT_PLAYER_X;
    lvl->player_spawn.y = DEFAULT_PLAYER_Y;
    lvl->enemy_spawn_count = N_DEFAULT_ENEMIES;
    for (int i = 0; i < N_DEFAULT_ENEMIES; i++) {
        lvl->enemy_spawns[i].x = DEFAULT_ENEMY_X[i];
        lvl->enemy_spawns[i].y = DEFAULT_ENEMY_Y[i];
    }
}

bool level_load(Level *lvl, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "level_load: cannot open '%s'\n", path);
        return false;
    }

    static char line_buf[LEVEL_ROWS_MAX][LEVEL_COLS_MAX + 2];
    const  char *line_ptrs[LEVEL_ROWS_MAX];
    int rows = 0, max_cols = 0;

    while (rows < LEVEL_ROWS_MAX &&
           fgets(line_buf[rows], (int)sizeof(line_buf[rows]), f)) {
        int len = (int)strlen(line_buf[rows]);
        while (len > 0 &&
               (line_buf[rows][len-1] == '\n' ||
                line_buf[rows][len-1] == '\r'))
            line_buf[rows][--len] = '\0';

        if (len == 0)               continue;
        if (line_buf[rows][0]==';') continue;

        if (len > max_cols) max_cols = len;
        line_ptrs[rows] = line_buf[rows];
        rows++;
    }
    fclose(f);

    if (rows == 0 || max_cols == 0) {
        fprintf(stderr, "level_load: '%s' is empty\n", path);
        return false;
    }

    parse_map_chars(lvl, rows, max_cols, line_ptrs);
    printf("level_load: '%s' — %d x %d, %d enemy spawn(s)\n",
           path, max_cols, rows, lvl->enemy_spawn_count);
    return true;
}

uint8_t level_tile_at(const Level *lvl, int col, int row) {
    if (col < 0 || col >= lvl->cols || row < 0 || row >= lvl->rows)
        return TILE_SOLID;
    return lvl->tiles[row][col];
}

bool level_is_solid(const Level *lvl, int col, int row) {
    return level_tile_at(lvl, col, row) == TILE_SOLID;
}

/* Only TILE_SOLID blocks movement — spring tiles are passable from below */
bool level_collides(const Level *lvl, AABB box,
                    int *out_col, int *out_row) {
    int left   = (int)(box.x)              / TILE_SIZE;
    int right  = (int)(box.x + box.w - 1) / TILE_SIZE;
    int top    = (int)(box.y)              / TILE_SIZE;
    int bottom = (int)(box.y + box.h - 1) / TILE_SIZE;

    for (int r = top; r <= bottom; r++) {
        for (int c = left; c <= right; c++) {
            if (level_tile_at(lvl, c, r) == TILE_SOLID) {
                if (out_col) *out_col = c;
                if (out_row) *out_row = r;
                return true;
            }
        }
    }
    return false;
}

bool level_on_spring(const Level *lvl, AABB box,
                     int *out_col, int *out_row) {
    /* Check the row of tiles just below the entity's feet */
    int left   = (int)(box.x)              / TILE_SIZE;
    int right  = (int)(box.x + box.w - 1) / TILE_SIZE;
    int bottom = (int)(box.y + box.h)      / TILE_SIZE;

    for (int c = left; c <= right; c++) {
        if (level_tile_at(lvl, c, bottom) == TILE_SPRING) {
            if (out_col) *out_col = c;
            if (out_row) *out_row = bottom;
            return true;
        }
    }
    return false;
}

void level_render(const Level *lvl, SDL_Renderer *renderer,
                  SDL_Texture *tile_tex, SDL_Texture *spring_tex) {
    for (int r = 0; r < lvl->rows; r++) {
        for (int c = 0; c < lvl->cols; c++) {
            uint8_t t = lvl->tiles[r][c];
            if (t == TILE_AIR) continue;

            SDL_Rect dst = { c * TILE_SIZE, r * TILE_SIZE,
                             TILE_SIZE, TILE_SIZE };

            if (t == TILE_SPRING) {
                if (spring_tex) {
                    SDL_RenderCopy(renderer, spring_tex, NULL, &dst);
                } else {
                    /* Fallback: bright green with coil lines */
                    SDL_SetRenderDrawColor(renderer, 60, 220, 80, 255);
                    SDL_RenderFillRect(renderer, &dst);
                    SDL_SetRenderDrawColor(renderer, 20, 140, 40, 255);
                    SDL_RenderDrawRect(renderer, &dst);
                    /* Draw a simple coil hint */
                    for (int i = 0; i < 3; i++) {
                        SDL_Rect coil = { dst.x + 6, dst.y + 6 + i*8, 20, 4 };
                        SDL_RenderFillRect(renderer, &coil);
                    }
                }
            } else {
                /* TILE_SOLID */
                if (tile_tex) {
                    SDL_RenderCopy(renderer, tile_tex, NULL, &dst);
                } else {
                    SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
                    SDL_RenderFillRect(renderer, &dst);
                    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
                    SDL_RenderDrawRect(renderer, &dst);
                }
            }
        }
    }
}
