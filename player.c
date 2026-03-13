#include "player.h"
#include <SDL2/SDL.h>

AABB player_aabb(const Player *p) {
    return (AABB){ p->x, p->y, PLAYER_W, PLAYER_H };
}

void player_init(Player *p, float start_x, float start_y) {
    p->x = start_x;
    p->y = start_y;
    p->vx = 0;
    p->vy = 0;
    p->on_ground = false;
    p->alive = true;
}

void player_handle_input(Player *p, const uint8_t *keys) {
    if (!p->alive) return;

    p->vx = 0;
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A])
        p->vx = -PLAYER_SPEED;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
        p->vx =  PLAYER_SPEED;

    if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] ||
         keys[SDL_SCANCODE_UP]) && p->on_ground) {
        p->vy = PLAYER_JUMP;
        p->on_ground = false;
    }
}

void player_update(Player *p, const Level *lvl, float dt) {
    if (!p->alive) return;

    /* apply gravity */
    p->vy += GRAVITY * dt;

    /* --- X axis --- */
    p->x += p->vx * dt;
    {
        int tc, tr;
        AABB box = player_aabb(p);
        if (level_collides(lvl, box, &tc, &tr)) {
            float tile_left  = (float)(tc * TILE_SIZE);
            float tile_right = tile_left + TILE_SIZE;
            if (p->vx > 0)
                p->x = tile_left - PLAYER_W;   /* push left */
            else if (p->vx < 0)
                p->x = tile_right;              /* push right */
            p->vx = 0;
        }
    }

    /* clamp to level horizontal bounds */
    if (p->x < 0) { p->x = 0; p->vx = 0; }
    if (p->x + PLAYER_W > lvl->pixel_width) {
        p->x = (float)(lvl->pixel_width - PLAYER_W);
        p->vx = 0;
    }

    /* --- Y axis --- */
    p->on_ground = false;
    p->y += p->vy * dt;
    {
        int tc, tr;
        AABB box = player_aabb(p);
        if (level_collides(lvl, box, &tc, &tr)) {
            float tile_top    = (float)(tr * TILE_SIZE);
            float tile_bottom = tile_top + TILE_SIZE;
            if (p->vy > 0) {
                /* landed on top of tile */
                p->y = tile_top - PLAYER_H;
                p->on_ground = true;
            } else {
                /* hit ceiling */
                p->y = tile_bottom;
            }
            p->vy = 0;
        }
    }

    /* fell off the bottom → respawn */
    if (p->y > lvl->pixel_height + 100) {
        player_init(p, 64, 400);
    }
}

void player_render(const Player *p, SDL_Renderer *renderer) {
    if (!p->alive) return;

    /* body */
    SDL_SetRenderDrawColor(renderer, 80, 200, 120, 255);
    SDL_Rect body = { (int)p->x, (int)p->y, PLAYER_W, PLAYER_H };
    SDL_RenderFillRect(renderer, &body);

    /* eyes */
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_Rect eye_l = { (int)p->x + 5,  (int)p->y + 8, 4, 4 };
    SDL_Rect eye_r = { (int)p->x + 15, (int)p->y + 8, 4, 4 };
    SDL_RenderFillRect(renderer, &eye_l);
    SDL_RenderFillRect(renderer, &eye_r);

    /* feet indicator when in the air */
    if (!p->on_ground) {
        SDL_SetRenderDrawColor(renderer, 60, 160, 90, 200);
        SDL_Rect feet = { (int)p->x + 4, (int)p->y + PLAYER_H - 6,
                          PLAYER_W - 8, 4 };
        SDL_RenderFillRect(renderer, &feet);
    }
}
