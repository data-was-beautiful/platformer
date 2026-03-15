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
    p->facing = 1;
    p->shoot_cooldown = 0.0f;
}

void player_handle_input(Player *p, const uint8_t *keys,
                         SDL_GameController *ctrl,
                         bool *shoot_requested) {
    if (!p->alive) return;

    /* --- Keyboard --- */
    bool kb_left  = keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A];
    bool kb_right = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    bool kb_jump  = keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_W] ||
                    keys[SDL_SCANCODE_UP];
    bool kb_shoot = keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL];

    /* --- Controller --- */
    bool ctrl_left = false, ctrl_right = false, ctrl_jump = false;
    bool ctrl_shoot = false;
    if (ctrl) {
        /* D-pad */
        ctrl_left  = SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_DPAD_LEFT)  != 0;
        ctrl_right = SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;
        ctrl_jump  = SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_A)          != 0 ||
                     SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_B)          != 0 ||
                     SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_DPAD_UP)    != 0;
        ctrl_shoot = SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_X)          != 0 ||
                     SDL_GameControllerGetButton(ctrl,
                         SDL_CONTROLLER_BUTTON_Y)          != 0;

        /* Left analog stick (dead-zone: 8000 / 32767 ≈ 24 %) */
        Sint16 axis_x = SDL_GameControllerGetAxis(ctrl,
                            SDL_CONTROLLER_AXIS_LEFTX);
        if (axis_x < -8000) ctrl_left  = true;
        if (axis_x >  8000) ctrl_right = true;
    }

    p->vx = 0;
    if (kb_left  || ctrl_left)  { p->vx = -PLAYER_SPEED; p->facing = -1; }
    if (kb_right || ctrl_right) { p->vx =  PLAYER_SPEED; p->facing =  1; }

    if ((kb_jump || ctrl_jump) && p->on_ground) {
        p->vy        = PLAYER_JUMP;
        p->on_ground = false;
    }

    /* Shoot: request a bullet if cooldown has expired */
    if (shoot_requested) {
        *shoot_requested = (kb_shoot || ctrl_shoot) &&
                           (p->shoot_cooldown <= 0.0f);
        if (*shoot_requested)
            p->shoot_cooldown = SHOOT_COOLDOWN;
    }
}

void player_update(Player *p, const Level *lvl, float dt) {
    if (!p->alive) return;

    /* Tick shoot cooldown */
    if (p->shoot_cooldown > 0.0f) {
        p->shoot_cooldown -= dt;
        if (p->shoot_cooldown < 0.0f) p->shoot_cooldown = 0.0f;
    }

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
                p->x = tile_left - PLAYER_W;
            else if (p->vx < 0)
                p->x = tile_right;
            p->vx = 0;
        }
    }

    if (p->x < 0) { p->x = 0; p->vx = 0; }
    if (p->x + PLAYER_W > lvl->pixel_width) {
        p->x = (float)(lvl->pixel_width - PLAYER_W);
        p->vx = 0;
    }

    /* --- Y axis --- */
    p->on_ground  = false;
    p->on_spring  = false;
    p->y += p->vy * dt;
    {
        int tc, tr;
        AABB box = player_aabb(p);
        if (level_collides(lvl, box, &tc, &tr)) {
            float tile_top    = (float)(tr * TILE_SIZE);
            float tile_bottom = tile_top + TILE_SIZE;
            if (p->vy > 0) {
                p->y = tile_top - PLAYER_H;
                p->on_ground = true;
            } else {
                p->y = tile_bottom;
            }
            p->vy = 0;
        }

        /* Spring launch — fires when feet touch a spring tile */
        AABB box2 = player_aabb(p);
        if (level_on_spring(lvl, box2, NULL, NULL)) {
            p->vy = -900.0f;   /* stronger than normal jump */
            p->on_ground = false;
            p->on_spring = true;
        }
    }

    if (p->y > lvl->pixel_height + 100)
        player_init(p, 64, 400);
}

void player_render(const Player *p, SDL_Renderer *renderer,
                   SDL_Texture *tex) {
    if (!p->alive) return;

    SDL_Rect dst = { (int)p->x, (int)p->y, PLAYER_W, PLAYER_H };

    if (tex) {
        /* Spritesheet: frame 0 = idle (left half), frame 1 = jump (right half).
           Each frame is PLAYER_W wide and PLAYER_H tall. */
        int frame = p->on_ground ? 0 : 1;
        SDL_Rect src = { frame * PLAYER_W, 0, PLAYER_W, PLAYER_H };

        /* Flip horizontally when moving left */
        SDL_RendererFlip flip = (p->vx < 0) ? SDL_FLIP_HORIZONTAL
                                             : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, tex, &src, &dst, 0.0, NULL, flip);
    } else {
        /* Fallback: coloured rectangles */
        SDL_SetRenderDrawColor(renderer, 80, 200, 120, 255);
        SDL_RenderFillRect(renderer, &dst);

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_Rect eye_l = { (int)p->x + 5,  (int)p->y + 8, 4, 4 };
        SDL_Rect eye_r = { (int)p->x + 15, (int)p->y + 8, 4, 4 };
        SDL_RenderFillRect(renderer, &eye_l);
        SDL_RenderFillRect(renderer, &eye_r);

        if (!p->on_ground) {
            SDL_SetRenderDrawColor(renderer, 60, 160, 90, 200);
            SDL_Rect feet = { (int)p->x + 4, (int)p->y + PLAYER_H - 6,
                              PLAYER_W - 8, 4 };
            SDL_RenderFillRect(renderer, &feet);
        }
    }
}
