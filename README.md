# C SDL2 Platformer

A 2D platformer written in C using SDL2. Built from scratch with no game engine —
just SDL2 and two optional companion libraries for images and audio.

---

## Features

- Title screen with keyboard-navigated menu (Start / Music toggle / Exit)
- 3 lives — lose them all and you return to the title screen
- Score counter — +1 per enemy stomped, displayed top-right
- Level progression — clear all enemies to advance to the next level file;
  loops back to level 1 if no further level file exists
- Tile-based levels loaded from plain-text `.txt` files — edit in any text editor,
  no recompile needed
- Patrol enemies with wall-bounce and ledge detection
- Stomp enemies by landing on them from above; touching them sideways costs a life
- Sprite rendering via `SDL_RenderCopy` / `SDL_RenderCopyEx` with automatic
  fallback to coloured rectangles if PNG files are absent
- Horizontal sprite flipping based on movement direction
- 2-frame player spritesheet (idle / airborne)
- Sound effects (jump, stomp, death) and looping background music via SDL2_mixer,
  with graceful fallback to silence if the library is absent
- Fixed-timestep physics loop at 60 Hz

---

## Controls

| Key | Action |
|-----|--------|
| `A` / `←` | Move left |
| `D` / `→` | Move right |
| `W` / `↑` / `Space` | Jump |
| `↑` / `↓` | Navigate title menu |
| `Enter` | Confirm menu selection |
| `Escape` (in-game) | Return to title screen |
| `Escape` (title) | Quit |

---

## Dependencies

The game has one required library and two optional ones:

| Library | Required | Purpose |
|---------|----------|---------|
| SDL2 | **Yes** | Window, renderer, input, game loop |
| SDL2_image | No | Load PNG sprite files |
| SDL2_mixer | No | Sound effects and music |

If SDL2_image is absent, sprites fall back to coloured rectangles.
If SDL2_mixer is absent, the game runs silently.
The Makefile detects both libraries automatically using `pkg-config` — no manual
flag editing needed.

---

## Installation & Build

### Linux (Debian / Ubuntu)

Install all three libraries:
```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

Build and run:
```bash
make
./platformer
```

To build with only the required library (no sprites, no audio):
```bash
sudo apt install libsdl2-dev
make
./platformer
```

### Linux (Arch / Manjaro)
```bash
sudo pacman -S sdl2 sdl2_image sdl2_mixer
make
./platformer
```

### Linux (Fedora / RHEL)
```bash
sudo dnf install SDL2-devel SDL2_image-devel SDL2_mixer-devel
make
./platformer
```

### macOS (Homebrew)

Install Homebrew if needed: https://brew.sh

```bash
brew install sdl2 sdl2_image sdl2_mixer
make
./platformer
```

### Windows (MSYS2 / MinGW-w64)

Install MSYS2 from https://www.msys2.org, then open the **MSYS2 MinGW 64-bit** shell:

```bash
pacman -S mingw-w64-x86_64-SDL2 \
          mingw-w64-x86_64-SDL2_image \
          mingw-w64-x86_64-SDL2_mixer \
          mingw-w64-x86_64-gcc \
          make
make
./platformer.exe
```

If you are using pre-built SDL2 development ZIPs from https://libsdl.org instead,
set the directory variables before running make:

```bash
make SDL2_DIR=C:/SDL2 SDL2_IMAGE_DIR=C:/SDL2_image SDL2_MIXER_DIR=C:/SDL2_mixer
```

### Verifying the build

When you run `make`, the Makefile prints which optional libraries were found:

```
SDL2_image  : enabled
SDL2_mixer  : enabled
```

or:

```
SDL2_image  : not found  [apt install libsdl2-image-dev]
SDL2_mixer  : not found  [apt install libsdl2-mixer-dev]
```

On first launch, the game also prints a diagnostics block to the terminal
showing the working directory and which assets loaded successfully.

---

## Project Structure

```
platformer/
├── main.c          — entry point, fixed-timestep game loop
├── game.h/.c       — top-level state machine, init, update, render, HUD
├── title.h/.c      — title screen: menu, bitmap font renderer
├── player.h/.c     — player entity: input, physics, collision, render
├── enemy.h/.c      — patrol enemy: AI, ledge detection, render
├── level.h/.c      — tilemap: file loading, collision queries, tile render
├── physics.h/.c    — shared AABB struct and overlap test
├── sprites.h/.c    — SDL2_image PNG loading with compile-time guard
├── audio.h/.c      — SDL2_mixer SFX + music with compile-time guard
├── Makefile
├── generate_placeholder_sprites.py
└── assets/
    ├── level1.txt       — level files, one per level
    ├── level2.txt
    ├── level3.txt
    ├── player.png       — optional sprite (see Sprites section)
    ├── enemy.png        — optional sprite
    ├── tile.png         — optional sprite
    ├── music.ogg        — optional background music
    ├── jump.wav         — optional SFX
    ├── stomp.wav        — optional SFX
    └── death.wav        — optional SFX
```

### Module responsibilities

**`game.h/.c`** owns the `AppState` enum (`STATE_TITLE`, `STATE_PLAYING`,
`STATE_QUIT`) and the top-level `Game` struct. All per-frame dispatch goes
through here. It also manages level loading, the score/lives system, and
the in-game HUD.

**`title.h/.c`** is fully self-contained. It renders the title screen using
a built-in 5×7 bitmap font (no SDL_ttf required) and handles all menu
navigation. The music toggle state is owned here and propagated back to `Game`.

**`sprites.h/.c`** and **`audio.h/.c`** are wrapped entirely in
`#ifdef SDL2_IMAGE_FOUND` / `#ifdef SDL2_MIXER_FOUND` guards. If the libraries
are not installed, the modules compile as no-op stubs and the rest of the
game is unaffected.

---

## Sprites

Sprite PNGs are loaded from `assets/` at startup. All are optional — the game
draws coloured rectangles if any file is missing.

| File | Dimensions | Format |
|------|-----------|--------|
| `assets/player.png` | 48 × 32 px | Spritesheet: left 24 px = idle frame, right 24 px = airborne frame |
| `assets/enemy.png` | 24 × 24 px | Single frame facing right; engine flips horizontally for left direction |
| `assets/tile.png` | 32 × 32 px | Tiled across every solid cell |

All files should be RGBA PNG. Transparency is supported.

### Generating placeholder sprites

A helper script creates simple but recognisable placeholder PNGs so you can
see textured rendering before you have real artwork:

```bash
pip install Pillow
python3 generate_placeholder_sprites.py
```

Replace the output files with your own art at any time — no recompile needed.

### Recommended pixel art tools

- **Aseprite** — https://www.aseprite.org (paid, purpose-built for sprites)
- **Libresprite** — https://libresprite.github.io (free Aseprite fork)
- **Piskel** — https://www.piskelapp.com (free, browser-based)

### Free sprite asset sources

- **Kenney.nl** — https://kenney.nl/assets — CC0 tilesets and character packs
- **OpenGameArt.org** — https://opengameart.org — CC0 and CC-BY sprites
- **itch.io** — https://itch.io/game-assets/free/tag-pixel-art — varied free packs

To resize a downloaded asset to the required dimensions using ImageMagick:
```bash
convert source.png -resize 32x32! assets/tile.png
```

---

## Audio

Audio files are loaded from `assets/` via SDL2_mixer. All are optional.

| File | Trigger |
|------|---------|
| `assets/music.ogg` | Loops continuously during gameplay |
| `assets/jump.wav` | Player jumps |
| `assets/stomp.wav` | Enemy stomped |
| `assets/death.wav` | Player loses a life |

WAV or OGG format work for all files. To use OGG for sound effects, rename
them and update `SFX_PATHS` in `audio.c`.

Music can be toggled on or off from the title screen menu. The setting persists
for the session. Volume levels are set in `game_init` in `game.c`:

```c
audio_set_music_volume(80);          /* 0–128 */
audio_set_sfx_volume(&g->audio, 110);
```

### Free audio sources

- **freesound.org** — https://freesound.org — CC0 and CC-BY sound effects
- **OpenGameArt.org** — https://opengameart.org — game-ready SFX and music
- **Kenney.nl** — https://kenney.nl/assets?q=audio — CC0 SFX packs
- **itch.io** — https://itch.io/game-assets/free/tag-sound-effects — curated packs

---

## Level Files

Levels are plain-text files in `assets/`. The game loads `level1.txt` on start,
then advances to `level2.txt`, `level3.txt`, and so on each time all enemies are
defeated. If the next file does not exist, it loops back to `level1.txt`.
The score carries over across levels.

### Tile characters

| Character | Meaning |
|-----------|---------|
| `#` | Solid tile |
| `.` | Air (empty space) |
| `P` | Player spawn point (first occurrence used) |
| `E` | Enemy spawn point |
| `;` | Comment — entire line ignored |
| blank line | Ignored |

### Dimensions

- Up to **80 columns × 40 rows** per level
- Each tile is 32 × 32 pixels
- Window is 1280 × 640 (40 × 20 tiles at default size)
- Level dimensions are read automatically from file content — no header needed
- Rows and columns outside the window bounds are valid (future camera support)

### Example level file

```
; My level — 40 cols x 20 rows
........................................
........................................
........#####...........................
........................................
.................#####..................
........................................
.....######.............................
........................................
..........................######........
........................................
..............########..................
........................................
........................................
...E....................................
..######..............E.................
.................E......................
...................######...............
........................................
P.......................................
########################################
```

### Adding new levels

Create `assets/level4.txt`, `assets/level5.txt`, etc. following the format above.
The game will pick them up automatically — no code changes required.

---

## Extending the Game

**Animations** — add `anim_frame` and `anim_timer` fields to the `Player` struct,
advance the frame on a fixed timer in `player_update`, and select the correct
source rect in `player_render`. Widen the spritesheet PNG to hold more frames.

**Scrolling camera** — add a `Camera { float x, y; }` to the `Game` struct, lerp
it toward the player each frame, and subtract it from all render positions. The
level and collision code already supports maps wider than the window.

**New tile types** — add constants (`TILE_SPIKE`, `TILE_LADDER`, etc.) to `level.h`,
handle them in `level_collides` for physics and in `level_render` for drawing.
Add corresponding characters to the level file parser in `level.c`.

**More lives / starting lives** — change `STARTING_LIVES` in `game.h`.

**High score / persistence** — write `score` to a file in `game_goto_title` and
read it back in `game_init` using standard `fopen` / `fprintf` / `fscanf`.
