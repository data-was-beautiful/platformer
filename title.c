#include "title.h"
#include <SDL2/SDL.h>
#include <string.h>

/* -------------------------------------------------------------------------
   We have no font library, so we render menu text as coloured blocks
   arranged to spell out labels — essentially a minimal bitmap-font
   approach using SDL_RenderFillRect for each "pixel" of each character.

   Each glyph is 5 columns × 7 rows, scaled up by GLYPH_SCALE.
   Only the characters we actually need are defined.
   -------------------------------------------------------------------------- */

#define GLYPH_W      5
#define GLYPH_H      7
#define GLYPH_SCALE  4          /* each pixel → 4×4 screen pixels */
#define GLYPH_GAP    2          /* gap between characters in pixels */

/* Glyph bitmaps — each uint8_t is one row, bits 4..0 = columns left→right */
typedef struct { uint8_t rows[GLYPH_H]; } Glyph;

static const Glyph GLYPHS[128] = {
    ['A'] = {{ 0x04,0x0A,0x11,0x1F,0x11,0x11,0x11 }},
    ['B'] = {{ 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E }},
    ['C'] = {{ 0x0E,0x11,0x10,0x10,0x10,0x11,0x0E }},
    ['D'] = {{ 0x1C,0x12,0x11,0x11,0x11,0x12,0x1C }},
    ['E'] = {{ 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F }},
    ['F'] = {{ 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 }},
    ['G'] = {{ 0x0E,0x11,0x10,0x17,0x11,0x11,0x0F }},
    ['H'] = {{ 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 }},
    ['I'] = {{ 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E }},
    ['J'] = {{ 0x07,0x02,0x02,0x02,0x02,0x12,0x0C }},
    ['K'] = {{ 0x11,0x12,0x14,0x18,0x14,0x12,0x11 }},
    ['L'] = {{ 0x10,0x10,0x10,0x10,0x10,0x10,0x1F }},
    ['M'] = {{ 0x11,0x1B,0x15,0x11,0x11,0x11,0x11 }},
    ['N'] = {{ 0x11,0x19,0x15,0x13,0x11,0x11,0x11 }},
    ['O'] = {{ 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E }},
    ['P'] = {{ 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 }},
    ['Q'] = {{ 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D }},
    ['R'] = {{ 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 }},
    ['S'] = {{ 0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E }},
    ['T'] = {{ 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 }},
    ['U'] = {{ 0x11,0x11,0x11,0x11,0x11,0x11,0x0E }},
    ['V'] = {{ 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 }},
    ['W'] = {{ 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 }},
    ['X'] = {{ 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 }},
    ['Y'] = {{ 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 }},
    ['Z'] = {{ 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F }},
    [' '] = {{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }},
    ['-'] = {{ 0x00,0x00,0x00,0x1F,0x00,0x00,0x00 }},
    ['>'] = {{ 0x10,0x08,0x04,0x02,0x04,0x08,0x10 }},
    ['0'] = {{ 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E }},
    ['1'] = {{ 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E }},
    ['2'] = {{ 0x0E,0x11,0x01,0x06,0x08,0x10,0x1F }},
    ['3'] = {{ 0x1F,0x01,0x02,0x06,0x01,0x11,0x0E }},
    ['4'] = {{ 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 }},
    ['5'] = {{ 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E }},
    ['6'] = {{ 0x06,0x08,0x10,0x1E,0x11,0x11,0x0E }},
    ['7'] = {{ 0x1F,0x01,0x02,0x04,0x04,0x04,0x04 }},
    ['8'] = {{ 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E }},
    ['9'] = {{ 0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C }},
    [':'] = {{ 0x00,0x04,0x00,0x00,0x00,0x04,0x00 }},
};

/* -------------------------------------------------------------------------
   Draw a single scaled glyph at (px, py) in the given colour
   ------------------------------------------------------------------------- */
static void draw_glyph(SDL_Renderer *r, char c,
                        int px, int py, SDL_Color col) {
    if ((unsigned char)c >= 128) return;
    const Glyph *g = &GLYPHS[(unsigned char)c];
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int row = 0; row < GLYPH_H; row++) {
        for (int col_bit = 0; col_bit < GLYPH_W; col_bit++) {
            if (g->rows[row] & (0x10 >> col_bit)) {
                SDL_Rect pixel = {
                    px + col_bit * GLYPH_SCALE,
                    py + row    * GLYPH_SCALE,
                    GLYPH_SCALE,
                    GLYPH_SCALE
                };
                SDL_RenderFillRect(r, &pixel);
            }
        }
    }
}

/* Draw a string; returns x position after last character */
static int draw_string(SDL_Renderer *r, const char *s,
                        int px, int py, SDL_Color col) {
    int x = px;
    int char_w = GLYPH_W * GLYPH_SCALE + GLYPH_GAP;
    for (int i = 0; s[i]; i++) {
        draw_glyph(r, s[i], x, py, col);
        x += char_w;
    }
    return x;
}

/* Width in pixels of a rendered string */
static int string_width(const char *s) {
    return (int)strlen(s) * (GLYPH_W * GLYPH_SCALE + GLYPH_GAP);
}

/* Draw a string centred at cx */
static void draw_string_centred(SDL_Renderer *r, const char *s,
                                  int cx, int py, SDL_Color col) {
    int x = cx - string_width(s) / 2;
    draw_string(r, s, x, py, col);
}

/* -------------------------------------------------------------------------
   Public API
   ------------------------------------------------------------------------- */
void title_init(TitleScreen *t, bool music_currently_on) {
    t->selected = MENU_START;
    t->music_on = music_currently_on;
}

bool title_handle_event(TitleScreen *t, SDL_Event *e,
                         SDL_GameController *ctrl,
                         bool *start_game, bool *quit,
                         bool *music_toggled) {
    *start_game    = false;
    *quit          = false;
    *music_toggled = false;

    /* --- Controller button events --- */
    if (e->type == SDL_CONTROLLERBUTTONDOWN) {
        switch (e->cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            t->selected = (t->selected + MENU_COUNT - 1) % MENU_COUNT;
            return false;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            t->selected = (t->selected + 1) % MENU_COUNT;
            return false;
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_START:
            goto confirm;
        case SDL_CONTROLLER_BUTTON_B:
        case SDL_CONTROLLER_BUTTON_BACK:
            *quit = true;
            return true;
        }
        return false;
    }

    /* --- Controller axis events (left stick up/down for menu nav) --- */
    if (e->type == SDL_CONTROLLERAXISMOTION &&
        e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
        if (e->caxis.value < -16000)
            t->selected = (t->selected + MENU_COUNT - 1) % MENU_COUNT;
        else if (e->caxis.value > 16000)
            t->selected = (t->selected + 1) % MENU_COUNT;
        return false;
    }

    /* --- Keyboard events --- */
    if (e->type != SDL_KEYDOWN) return false;

    switch (e->key.keysym.sym) {
    case SDLK_UP:
        t->selected = (t->selected + MENU_COUNT - 1) % MENU_COUNT;
        return false;
    case SDLK_DOWN:
        t->selected = (t->selected + 1) % MENU_COUNT;
        return false;

    case SDLK_RETURN:
    case SDLK_SPACE:
        goto confirm;

    case SDLK_ESCAPE:
        *quit = true;
        return true;
    }
    return false;

confirm:
    switch (t->selected) {
    case MENU_START:
        *start_game = true;
        return true;
    case MENU_MUSIC:
        t->music_on    = !t->music_on;
        *music_toggled = true;
        return false;
    case MENU_EXIT:
        *quit = true;
        return true;
    }
    return false;
}

void title_render(const TitleScreen *t, SDL_Renderer *renderer,
                  int window_w, int window_h) {
    int cx = window_w / 2;

    /* Background */
    SDL_SetRenderDrawColor(renderer, 10, 12, 28, 255);
    SDL_RenderClear(renderer);

    /* Decorative bottom strip */
    SDL_SetRenderDrawColor(renderer, 20, 22, 50, 255);
    SDL_Rect strip = { 0, window_h * 2 / 3, window_w, window_h };
    SDL_RenderFillRect(renderer, &strip);

    /* Title — large, two-tone */
    SDL_Color title_col  = { 255, 220, 60,  255 };
    SDL_Color shadow_col = { 120, 80,  0,   255 };

    /* Scale title up 3× by drawing it at 3× glyph scale */
    /* We'll just draw it twice offset for a shadow effect */
    const char *title = "PLATFORMER";
    int title_big_w = (int)strlen(title) * (GLYPH_W * GLYPH_SCALE * 2 + GLYPH_GAP);
    int title_x = cx - title_big_w / 2;
    int title_y = window_h / 5;

    /* Shadow pass */
    SDL_SetRenderDrawColor(renderer, shadow_col.r, shadow_col.g,
                           shadow_col.b, shadow_col.a);
    for (int i = 0; title[i]; i++) {
        int px = title_x + i * (GLYPH_W * GLYPH_SCALE * 2 + GLYPH_GAP);
        const Glyph *g = &GLYPHS[(unsigned char)title[i]];
        for (int row = 0; row < GLYPH_H; row++)
            for (int cb = 0; cb < GLYPH_W; cb++)
                if (g->rows[row] & (0x10 >> cb)) {
                    SDL_Rect pixel = {
                        px + cb * GLYPH_SCALE * 2 + 4,
                        title_y + row * GLYPH_SCALE * 2 + 4,
                        GLYPH_SCALE * 2,
                        GLYPH_SCALE * 2
                    };
                    SDL_RenderFillRect(renderer, &pixel);
                }
    }

    /* Title pass */
    SDL_SetRenderDrawColor(renderer, title_col.r, title_col.g,
                           title_col.b, title_col.a);
    for (int i = 0; title[i]; i++) {
        int px = title_x + i * (GLYPH_W * GLYPH_SCALE * 2 + GLYPH_GAP);
        const Glyph *g = &GLYPHS[(unsigned char)title[i]];
        for (int row = 0; row < GLYPH_H; row++)
            for (int cb = 0; cb < GLYPH_W; cb++)
                if (g->rows[row] & (0x10 >> cb)) {
                    SDL_Rect pixel = {
                        px + cb * GLYPH_SCALE * 2,
                        title_y + row * GLYPH_SCALE * 2,
                        GLYPH_SCALE * 2,
                        GLYPH_SCALE * 2
                    };
                    SDL_RenderFillRect(renderer, &pixel);
                }
    }

    /* Subtitle */
    SDL_Color sub_col = { 160, 160, 200, 255 };
    draw_string_centred(renderer, "ARROW KEYS  ENTER  OR  CONTROLLER",
                        cx, title_y + GLYPH_H * GLYPH_SCALE * 2 + 20, sub_col);

    /* Menu items */
    int menu_y_start = window_h / 2 + 20;
    int menu_spacing = GLYPH_H * GLYPH_SCALE + 24;

    const char *music_label = t->music_on ? "MUSIC  ON" : "MUSIC  OFF";
    const char *labels[MENU_COUNT] = {
        "START GAME",
        music_label,
        "EXIT GAME"
    };
    SDL_Color col_normal   = { 180, 180, 210, 255 };
    SDL_Color col_selected = { 255, 255, 100, 255 };
    SDL_Color col_cursor   = { 255, 200, 0,   255 };

    for (int i = 0; i < MENU_COUNT; i++) {
        int item_y = menu_y_start + i * menu_spacing;
        SDL_Color col = (i == t->selected) ? col_selected : col_normal;

        /* Cursor arrow */
        if (i == t->selected) {
            draw_glyph(renderer, '>',
                       cx - string_width(labels[i]) / 2 - (GLYPH_W * GLYPH_SCALE + GLYPH_GAP) - 8,
                       item_y, col_cursor);
        }

        draw_string_centred(renderer, labels[i], cx, item_y, col);
    }

    /* Footer hint */
    SDL_Color hint_col = { 80, 80, 110, 255 };
    draw_string_centred(renderer, "ESC TO QUIT",
                        cx, window_h - 40, hint_col);
}
