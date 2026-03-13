#ifndef AUDIO_H
#define AUDIO_H

/*
 * Audio module — wraps SDL2_mixer for sound effects and music.
 * Compiled out entirely if SDL2_MIXER_FOUND is not defined.
 * All functions are safe to call regardless — they no-op when
 * the library is absent or a file fails to load.
 *
 * Expected assets:
 *   assets/music.ogg   — looping background track
 *   assets/jump.wav    — player jump
 *   assets/stomp.wav   — enemy stomped
 *   assets/death.wav   — player killed by enemy
 */

/* Sound effect identifiers */
typedef enum {
    SFX_JUMP  = 0,
    SFX_STOMP = 1,
    SFX_DEATH = 2,
    SFX_COUNT = 3
} SfxId;

typedef struct {
#ifdef SDL2_MIXER_FOUND
    struct Mix_Chunk *sfx[SFX_COUNT];
    /* Music is managed internally by SDL2_mixer */
#else
    int _dummy;   /* keep struct non-empty in C */
#endif
} Audio;

/* Initialise mixer, load all audio files. Logs warnings on failure. */
void audio_load(Audio *a);

/* Play a one-shot sound effect. Safe to call with any SfxId. */
void audio_play_sfx(Audio *a, SfxId id);

/* Start / stop the background music track. */
void audio_play_music(Audio *a);
void audio_stop_music(void);

/* Pause / resume music (e.g. on window focus loss). */
void audio_pause_music(void);
void audio_resume_music(void);

/* Set volumes 0–128. MIX_MAX_VOLUME = 128. */
void audio_set_sfx_volume(Audio *a, int vol);
void audio_set_music_volume(int vol);

/* Free all resources. */
void audio_free(Audio *a);

#endif /* AUDIO_H */
