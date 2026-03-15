# -----------------------------------------------------------------------
#  Platformer — Makefile
#  Supports: Linux, macOS, Windows (MinGW/MSYS2)
#  SDL2_image and SDL2_mixer are optional — game falls back gracefully.
# -----------------------------------------------------------------------

TARGET  = platformer
SRCS    = main.c game.c player.c enemy.c level.c physics.c sprites.c audio.c title.c lootbox.c bullet.c
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

    ifneq ($(shell pkg-config --exists SDL2_image 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_IMAGE_FOUND $(shell pkg-config --cflags SDL2_image)
        LDFLAGS += $(shell pkg-config --libs SDL2_image)
        $(info SDL2_image  : enabled)
    else
        $(info SDL2_image  : not found  [apt install libsdl2-image-dev])
    endif

    ifneq ($(shell pkg-config --exists SDL2_mixer 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_MIXER_FOUND $(shell pkg-config --cflags SDL2_mixer)
        LDFLAGS += $(shell pkg-config --libs SDL2_mixer)
        $(info SDL2_mixer  : enabled)
    else
        $(info SDL2_mixer  : not found  [apt install libsdl2-mixer-dev])
    endif
endif

ifeq ($(UNAME), Darwin)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS  = $(shell sdl2-config --libs)

    ifneq ($(shell pkg-config --exists SDL2_image 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_IMAGE_FOUND $(shell pkg-config --cflags SDL2_image)
        LDFLAGS += $(shell pkg-config --libs SDL2_image)
        $(info SDL2_image  : enabled)
    else
        $(info SDL2_image  : not found  [brew install sdl2_image])
    endif

    ifneq ($(shell pkg-config --exists SDL2_mixer 2>/dev/null && echo yes),)
        CFLAGS  += -DSDL2_MIXER_FOUND $(shell pkg-config --cflags SDL2_mixer)
        LDFLAGS += $(shell pkg-config --libs SDL2_mixer)
        $(info SDL2_mixer  : enabled)
    else
        $(info SDL2_mixer  : not found  [brew install sdl2_mixer])
    endif
endif

ifeq ($(UNAME), Windows)
    SDL2_DIR ?= C:/SDL2
    CFLAGS   += -I$(SDL2_DIR)/include/SDL2
    LDFLAGS   = -L$(SDL2_DIR)/lib -lSDL2main -lSDL2 -mwindows
    TARGET    = platformer.exe

    ifneq ($(SDL2_IMAGE_DIR),)
        CFLAGS  += -DSDL2_IMAGE_FOUND -I$(SDL2_IMAGE_DIR)/include
        LDFLAGS += -L$(SDL2_IMAGE_DIR)/lib -lSDL2_image
        $(info SDL2_image  : enabled)
    else
        $(info SDL2_image  : disabled  [set SDL2_IMAGE_DIR=... to enable])
    endif

    ifneq ($(SDL2_MIXER_DIR),)
        CFLAGS  += -DSDL2_MIXER_FOUND -I$(SDL2_MIXER_DIR)/include
        LDFLAGS += -L$(SDL2_MIXER_DIR)/lib -lSDL2_mixer
        $(info SDL2_mixer  : enabled)
    else
        $(info SDL2_mixer  : disabled  [set SDL2_MIXER_DIR=... to enable])
    endif
endif

# -----------------------------------------------------------------------
#  Rules
# -----------------------------------------------------------------------
all: assets $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

assets:
	@mkdir -p assets

clean:
	rm -f $(OBJS) $(TARGET) platformer.exe

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run assets
