#include "audio.h"
#include <stdio.h>
#include <string.h>

#ifdef SDL2_MIXER_FOUND
#include <SDL2/SDL_mixer.h>

/* File paths for each sound effect, indexed by SfxId */
static const char *SFX_PATHS[SFX_COUNT] = {
    "assets/jump.wav",   /* SFX_JUMP  */
    "assets/stomp.wav",  /* SFX_STOMP */
    "assets/death.wav",  /* SFX_DEATH */
};

static const char *MUSIC_PATH = "assets/music.ogg";

/* Music handle — kept as a file-scope pointer so play/stop/pause
   don't need the Audio struct passed in. Only one track at a time. */
static Mix_Music *s_music = NULL;

/* -------------------------------------------------------------------------
   Internal helpers
   ------------------------------------------------------------------------- */
static Mix_Chunk *load_chunk(const char *path) {
    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk)
        fprintf(stderr, "WARNING audio: could not load '%s': %s\n",
                path, Mix_GetError());
    return chunk;   /* NULL on failure — callers handle gracefully */
}

/* -------------------------------------------------------------------------
   Public API — SDL2_mixer available
   ------------------------------------------------------------------------- */
void audio_load(Audio *a) {
    memset(a, 0, sizeof(*a));

    /* Initialise SDL2_mixer: 44100 Hz, stereo, 2048-byte chunks */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "WARNING audio: Mix_OpenAudio failed: %s\n",
                Mix_GetError());
        return;
    }

    /* Enable OGG support for music */
    int flags = Mix_Init(MIX_INIT_OGG);
    if (!(flags & MIX_INIT_OGG))
        fprintf(stderr, "WARNING audio: OGG support unavailable: %s\n",
                Mix_GetError());

    /* Allocate 8 mixing channels (enough for overlapping SFX) */
    Mix_AllocateChannels(8);

    /* Load sound effects */
    for (int i = 0; i < SFX_COUNT; i++)
        a->sfx[i] = load_chunk(SFX_PATHS[i]);

    /* Load music (streamed, not kept in RAM) */
    s_music = Mix_LoadMUS(MUSIC_PATH);
    if (!s_music)
        fprintf(stderr, "WARNING audio: could not load '%s': %s\n",
                MUSIC_PATH, Mix_GetError());

    fprintf(stderr, "INFO: audio — jump:%s stomp:%s death:%s music:%s\n",
            a->sfx[SFX_JUMP]  ? "OK" : "missing",
            a->sfx[SFX_STOMP] ? "OK" : "missing",
            a->sfx[SFX_DEATH] ? "OK" : "missing",
            s_music           ? "OK" : "missing");
}

void audio_play_sfx(Audio *a, SfxId id) {
    if (id < 0 || id >= SFX_COUNT) return;
    if (!a->sfx[id]) return;
    /* -1 = first free channel, 0 = play once */
    Mix_PlayChannel(-1, a->sfx[id], 0);
}

void audio_play_music(Audio *a) {
    (void)a;
    if (!s_music) return;
    /* -1 = loop forever */
    if (Mix_PlayMusic(s_music, -1) < 0)
        fprintf(stderr, "WARNING audio: Mix_PlayMusic: %s\n", Mix_GetError());
}

void audio_stop_music(void) {
    Mix_HaltMusic();
}

void audio_pause_music(void) {
    if (Mix_PlayingMusic()) Mix_PauseMusic();
}

void audio_resume_music(void) {
    if (Mix_PausedMusic()) Mix_ResumeMusic();
}

void audio_set_sfx_volume(Audio *a, int vol) {
    for (int i = 0; i < SFX_COUNT; i++)
        if (a->sfx[i]) Mix_VolumeChunk(a->sfx[i], vol);
}

void audio_set_music_volume(int vol) {
    Mix_VolumeMusic(vol);
}

void audio_free(Audio *a) {
    for (int i = 0; i < SFX_COUNT; i++) {
        if (a->sfx[i]) Mix_FreeChunk(a->sfx[i]);
    }
    if (s_music) { Mix_FreeMusic(s_music); s_music = NULL; }
    Mix_CloseAudio();
    Mix_Quit();
    memset(a, 0, sizeof(*a));
}

#else  /* ----------------------------------------------------------------
          SDL2_mixer not compiled in — safe no-op stubs
          -------------------------------------------------------------- */

void audio_load(Audio *a) {
    memset(a, 0, sizeof(*a));
    fprintf(stderr, "INFO: SDL2_mixer not compiled in — audio disabled\n");
}
void audio_play_sfx(Audio *a, SfxId id) { (void)a; (void)id; }
void audio_play_music(Audio *a)         { (void)a; }
void audio_stop_music(void)             {}
void audio_pause_music(void)            {}
void audio_resume_music(void)           {}
void audio_set_sfx_volume(Audio *a, int vol) { (void)a; (void)vol; }
void audio_set_music_volume(int vol)    { (void)vol; }
void audio_free(Audio *a)               { (void)a; }

#endif /* SDL2_MIXER_FOUND */
