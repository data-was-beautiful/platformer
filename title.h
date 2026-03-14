#ifndef TITLE_H
#define TITLE_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/* Menu items on the title screen */
typedef enum {
    MENU_START  = 0,
    MENU_MUSIC  = 1,
    MENU_EXIT   = 2,
    MENU_COUNT  = 3
} MenuItem;

typedef struct {
    int       selected;      /* currently highlighted item  */
    bool      music_on;      /* toggled by MENU_MUSIC       */
} TitleScreen;

void title_init(TitleScreen *t, bool music_currently_on);

/* Returns true when the user confirms a selection (Enter/Space/A button).
   Sets *start_game=true if they chose Start, *quit=true if Exit.
   Pass ctrl=NULL if no controller is connected. */
bool title_handle_event(TitleScreen *t, SDL_Event *e,
                        SDL_GameController *ctrl,
                        bool *start_game, bool *quit,
                        bool *music_toggled);

void title_render(const TitleScreen *t, SDL_Renderer *renderer,
                  int window_w, int window_h);

#endif
