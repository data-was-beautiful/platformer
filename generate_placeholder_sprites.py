#!/usr/bin/env python3
"""
generate_placeholder_sprites.py
Generates minimal placeholder PNG sprites so the game runs with
SDL_RenderCopy before you have real artwork.

Requires Pillow:  pip install Pillow

Output files (written to assets/):
  player.png  — 48x32  (2 frames: idle left half, jump right half)
  enemy.png   — 24x24  (single frame)
  tile.png    — 32x32  (single frame)
"""

import os
from PIL import Image, ImageDraw

ASSETS = os.path.join(os.path.dirname(__file__), "assets")
os.makedirs(ASSETS, exist_ok=True)

# ---------------------------------------------------------------------------
# Player spritesheet: 48x32  (frame 0 = idle | frame 1 = jump)
# ---------------------------------------------------------------------------
player = Image.new("RGBA", (48, 32), (0, 0, 0, 0))
d = ImageDraw.Draw(player)

for fx in (0, 24):                          # two frames, side by side
    # body
    d.rectangle([fx+2, 4, fx+21, 31], fill=(80, 200, 120, 255))
    # eyes
    d.rectangle([fx+5, 8, fx+8, 11],  fill=(20, 20, 20, 255))
    d.rectangle([fx+15, 8, fx+18, 11], fill=(20, 20, 20, 255))
    # mouth
    d.rectangle([fx+7, 20, fx+16, 22], fill=(20, 20, 20, 255))

# Jump frame: raised arms (slightly different shade)
d.rectangle([0, 0, 1, 31], fill=(0, 0, 0, 0))          # clear left edge
d.rectangle([24+2, 4, 24+21, 28], fill=(60, 180, 100, 255))   # squished body
d.rectangle([24+5, 7, 24+8, 10],   fill=(20, 20, 20, 255))
d.rectangle([24+15, 7, 24+18, 10], fill=(20, 20, 20, 255))

player.save(os.path.join(ASSETS, "player.png"))
print("Wrote assets/player.png (48x32)")

# ---------------------------------------------------------------------------
# Enemy: 24x24
# ---------------------------------------------------------------------------
enemy = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
d = ImageDraw.Draw(enemy)
d.rectangle([1, 1, 22, 22], fill=(210, 60, 60, 255), outline=(140, 20, 20, 255))
# eye (facing right by default — engine flips for left direction)
d.rectangle([14, 6, 18, 10], fill=(255, 220, 0, 255))
# angry brow
d.line([13, 4, 19, 6], fill=(80, 0, 0, 255), width=2)

enemy.save(os.path.join(ASSETS, "enemy.png"))
print("Wrote assets/enemy.png (24x24)")

# ---------------------------------------------------------------------------
# Tile: 32x32  — stone-ish look
# ---------------------------------------------------------------------------
tile = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
d = ImageDraw.Draw(tile)
# base fill
d.rectangle([0, 0, 31, 31], fill=(110, 110, 125, 255))
# highlight top-left
d.line([0, 0, 31, 0],  fill=(160, 160, 175, 255), width=2)
d.line([0, 0, 0, 31],  fill=(160, 160, 175, 255), width=2)
# shadow bottom-right
d.line([0, 31, 31, 31], fill=(70, 70, 82, 255), width=2)
d.line([31, 0, 31, 31], fill=(70, 70, 82, 255), width=2)
# subtle interior crack lines
d.line([8, 0, 8, 31],   fill=(90, 90, 103, 120), width=1)
d.line([20, 0, 20, 31],  fill=(90, 90, 103, 120), width=1)
d.line([0, 16, 31, 16],  fill=(90, 90, 103, 120), width=1)

tile.save(os.path.join(ASSETS, "tile.png"))
print("Wrote assets/tile.png (32x32)")

print("Done. Run 'make && ./platformer' to play with placeholder sprites.")
print("Replace the PNGs in assets/ with your real artwork any time.")
