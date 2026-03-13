#include "sprites.h"
#include <stdio.h>
#include <string.h>

#ifdef SDL2_IMAGE_FOUND
#include <SDL2/SDL_image.h>

static SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        fprintf(stderr, "WARNING: could not load '%s': %s\n",
                path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex)
        fprintf(stderr, "WARNING: CreateTextureFromSurface('%s'): %s\n",
                path, SDL_GetError());
    return tex;
}

void sprites_load(Sprites *s, SDL_Renderer *renderer) {
    memset(s, 0, sizeof(*s));
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "WARNING: IMG_Init failed: %s — sprites disabled\n",
                IMG_GetError());
        return;
    }
    s->player  = load_texture(renderer, "assets/player.png");
    s->enemy   = load_texture(renderer, "assets/enemy.png");
    s->tile    = load_texture(renderer, "assets/tile.png");
    s->spring  = load_texture(renderer, "assets/spring.png");
    s->lootbox = load_texture(renderer, "assets/lootbox.png");
    s->boss    = load_texture(renderer, "assets/boss.png");
}

void sprites_free(Sprites *s) {
    if (s->player)  SDL_DestroyTexture(s->player);
    if (s->enemy)   SDL_DestroyTexture(s->enemy);
    if (s->tile)    SDL_DestroyTexture(s->tile);
    if (s->spring)  SDL_DestroyTexture(s->spring);
    if (s->lootbox) SDL_DestroyTexture(s->lootbox);
    if (s->boss)    SDL_DestroyTexture(s->boss);
    IMG_Quit();
    memset(s, 0, sizeof(*s));
}

#else

void sprites_load(Sprites *s, SDL_Renderer *renderer) {
    (void)renderer;
    memset(s, 0, sizeof(*s));
    fprintf(stderr, "INFO: SDL2_image not compiled in — using rect fallback\n");
}

void sprites_free(Sprites *s) {
    memset(s, 0, sizeof(*s));
}

#endif
