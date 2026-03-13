# C SDL2 Platformer

A basic 2D platformer written in C using SDL2 + SDL2_image.

## Features
- Player movement, jumping, gravity
- Tile-based levels loaded from plain-text `.txt` files
- Sprite rendering via `SDL_RenderCopy` / `SDL_RenderCopyEx` (with rect fallback)
- Horizontal sprite flipping based on movement direction
- 2-frame player spritesheet (idle / jump)
- Patrol enemies with ledge detection and direction flip
- Stomp enemies by landing on top — bounce on kill
- Fixed-timestep game loop (60 Hz physics)
- Death counter HUD

## Controls
| Key | Action |
|-----|--------|
| A / ← | Move left |
| D / → | Move right |
| W / ↑ / Space | Jump |
| R | Restart / respawn |
| Escape | Quit |

## Build

### Linux
```bash
sudo apt install libsdl2-dev libsdl2-image-dev
make
./platformer
```

### macOS
```bash
brew install sdl2 sdl2_image
make
./platformer
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image
make
./platformer.exe
```

## Sprites

Sprite PNGs are loaded from `assets/` at startup.
**They are optional** — if any file is missing the game falls back to
coloured rectangles automatically.

| File | Size | Description |
|------|------|-------------|
| `assets/player.png` | 48×32 | 2-frame spritesheet: frame 0 idle (left 24px), frame 1 jump (right 24px) |
| `assets/enemy.png`  | 24×24 | Single frame; engine flips horizontally for direction |
| `assets/tile.png`   | 32×32 | Single tile, tiled across all solid cells |

### Generating placeholder sprites
```bash
pip install Pillow
python3 generate_placeholder_sprites.py
```
This writes simple coloured placeholder PNGs to `assets/` so you can
see textured rendering before you have real art. Replace the files any
time — no recompile needed.

## Level files

Levels are plain-text files loaded from `assets/level1.txt`
(configurable via `LEVEL_FILE` in `game.h`).

### Tile characters
| Char | Meaning |
|------|---------|
| `#`  | Solid tile |
| `.`  | Air (empty) |
| `P`  | Player spawn (first occurrence used) |
| `E`  | Enemy spawn |
| `;`  | Comment line (ignored) |
| blank line | Ignored |

### Dimensions
- Up to **80 columns × 40 rows** (configurable via `LEVEL_COLS_MAX` / `LEVEL_ROWS_MAX` in `level.h`)
- Each tile is `TILE_SIZE` pixels (32 by default)
- Rows and columns are determined automatically from file content — no header needed

### Fallback
If `assets/level1.txt` is missing or unreadable, the game loads a
built-in hardcoded map silently.

## Project Structure
```
platformer/
├── main.c        — entry point, fixed-timestep game loop
├── game.h/.c     — top-level state, init, update, render
├── player.h/.c   — player entity: input, physics, sprite render
├── enemy.h/.c    — patrol enemy: AI, ledge detection, sprite render
├── level.h/.c    — tilemap: file loading, collision, tile render
├── physics.h/.c  — shared AABB struct + overlap test
├── sprites.h/.c  — SDL2_image texture loading + graceful fallback
├── Makefile
├── generate_placeholder_sprites.py
└── assets/
    ├── level1.txt
    ├── player.png   ← optional, generate with script
    ├── enemy.png    ← optional
    └── tile.png     ← optional
```

## Tips for extending
- **New levels**: copy `assets/level1.txt`, edit it, change `LEVEL_FILE` in `game.h`
- **Animations**: extend the spritesheet width with more frames, track `anim_frame` in the entity struct, advance it on a timer
- **Camera**: add a `Camera {float x,y;}` to `Game`, subtract it from all render positions, lerp it toward the player each frame
- **Tile variety**: add tile types (e.g. `TILE_SPIKE = 2`) in `level.h`, handle them in `level_collides` and `level_render`
