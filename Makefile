# -----------------------------------------------------------------------
#  Platformer — Makefile
#  Supports: Linux, macOS, Windows (MinGW/MSYS2)
#  SDL2_image is optional — game falls back to coloured rects without it.
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
    LDFLAGS  = $(shell sdl2-config --libs)

    # Auto-detect SDL2_image
    ifneq ($(shell pkg-config --exists SDL2_image 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_IMAGE_FOUND $(shell pkg-config --cflags SDL2_image)
        LDFLAGS += $(shell pkg-config --libs SDL2_image)
        $(info SDL2_image found — sprites enabled)
    else
        $(info SDL2_image not found — install libsdl2-image-dev for sprites)
    endif
endif

ifeq ($(UNAME), Darwin)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS  = $(shell sdl2-config --libs)

    # Auto-detect SDL2_image (Homebrew)
    ifneq ($(shell pkg-config --exists SDL2_image 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_IMAGE_FOUND $(shell pkg-config --cflags SDL2_image)
        LDFLAGS += $(shell pkg-config --libs SDL2_image)
        $(info SDL2_image found — sprites enabled)
    else
        $(info SDL2_image not found — run: brew install sdl2_image)
    endif
endif

ifeq ($(UNAME), Windows)
    SDL2_DIR ?= C:/SDL2
    CFLAGS   += -I$(SDL2_DIR)/include/SDL2
    LDFLAGS   = -L$(SDL2_DIR)/lib -lSDL2main -lSDL2 -mwindows
    TARGET    = platformer.exe

    # Set SDL2_IMAGE_DIR to enable sprites on Windows
    ifneq ($(SDL2_IMAGE_DIR),)
        CFLAGS  += -DSDL2_IMAGE_FOUND -I$(SDL2_IMAGE_DIR)/include
        LDFLAGS += -L$(SDL2_IMAGE_DIR)/lib -lSDL2_image
        $(info SDL2_image found — sprites enabled)
    else
        $(info SDL2_image disabled — set SDL2_IMAGE_DIR=... to enable)
    endif
endif

# -----------------------------------------------------------------------
#  Rules
# -----------------------------------------------------------------------
all: assets $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

assets:
	@mkdir -p assets

clean:
	rm -f $(OBJS) $(TARGET) platformer.exe

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run assets
