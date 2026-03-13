# -----------------------------------------------------------------------
#  Platformer — Makefile
#  Supports: Linux, macOS, Windows (MinGW/MSYS2)
# -----------------------------------------------------------------------

TARGET  = platformer

SRCS    = main.c game.c player.c enemy.c level.c physics.c sprites.c
OBJS    = $(SRCS:.c=.o)

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2

# -----------------------------------------------------------------------
#  Platform detection
# -----------------------------------------------------------------------
UNAME := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME), Linux)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS  = $(shell sdl2-config --libs) -lSDL2_image
endif

ifeq ($(UNAME), Darwin)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS  = $(shell sdl2-config --libs) -lSDL2_image
endif

ifeq ($(UNAME), Windows)
    # MinGW / MSYS2 — adjust SDL2_DIR if needed
    SDL2_DIR ?= C:/SDL2
    CFLAGS   += -I$(SDL2_DIR)/include/SDL2
    LDFLAGS   = -L$(SDL2_DIR)/lib -lSDL2main -lSDL2 -lSDL2_image -mwindows
    TARGET    = platformer.exe
endif

# -----------------------------------------------------------------------
#  Rules
# -----------------------------------------------------------------------
all: assets $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create assets directory if missing (sprites are optional)
assets:
	@mkdir -p assets

clean:
	rm -f $(OBJS) $(TARGET) platformer.exe

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run assets
