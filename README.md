# C SDL2 Platformer

A basic 2D platformer written in C using SDL2.

## Features
- Player movement, jumping, gravity
- Tile-based level with platforms at various heights
- 8 patrolling enemies with ledge detection
- Stomp enemies by landing on them from above
- Fixed-timestep game loop (60 Hz physics)
- Death counter HUD (red squares, top-left)

## Controls
| Key | Action |
|-----|--------|
| A / ← | Move left |
| D / → | Move right |
| W / ↑ / Space | Jump |
| R | Restart |
| Escape | Quit |

## Build

### Linux
```bash
sudo apt install libsdl2-dev   # Debian/Ubuntu
# or: sudo pacman -S sdl2      # Arch
make
./platformer
```

### macOS
```bash
brew install sdl2
make
./platformer
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-SDL2
make
./platformer.exe
```

## Project Structure
```
platformer/
├── main.c       — entry point, fixed-timestep game loop
├── game.h/.c    — top-level game state, update, render
├── player.h/.c  — player entity, input, physics, collision
├── enemy.h/.c   — patrol enemy, ledge/wall detection
├── level.h/.c   — tilemap, solid-tile collision queries
├── physics.h/.c — shared AABB struct + overlap test
└── Makefile
```

## Tips for extending
- **New levels**: edit the `MAP` array in `level.c`
- **Tile size**: change `TILE_SIZE` in `level.h` (update window size too)
- **Sprites**: replace `SDL_RenderFillRect` calls with `SDL_RenderCopy` + textures
- **Camera**: offset all render positions by a camera struct tracking the player
- **More enemies**: increase `MAX_ENEMIES` in `enemy.h`
