#include "audio.h"
#include <stdio.h>
#include <string.h>

#ifdef SDL2_MIXER_FOUND
#include <SDL2/SDL_mixer.h>

static const char *SFX_PATHS[SFX_COUNT] = {
    "assets/jump.wav",          /* SFX_JUMP          */
    "assets/stomp.wav",         /* SFX_STOMP         */
    "assets/death.wav",         /* SFX_DEATH         */
    "assets/spring.wav",        /* SFX_SPRING        */
    "assets/lootbox_spawn.wav", /* SFX_LOOTBOX_SPAWN */
    "assets/lootbox_open.wav",  /* SFX_LOOTBOX_OPEN  */
    "assets/boss_spawn.wav",    /* SFX_BOSS_SPAWN    */
};

static const char *MUSIC_PATH = "assets/music.ogg";
static Mix_Music  *s_music    = NULL;

static Mix_Chunk *load_chunk(const char *path) {
    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk)
        fprintf(stderr, "WARNING audio: could not load '%s': %s\n",
                path, Mix_GetError());
    return chunk;
}

void audio_load(Audio *a) {
    memset(a, 0, sizeof(*a));
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "WARNING audio: Mix_OpenAudio: %s\n", Mix_GetError());
        return;
    }
    int flags = Mix_Init(MIX_INIT_OGG);
    if (!(flags & MIX_INIT_OGG))
        fprintf(stderr, "WARNING audio: OGG support unavailable: %s\n",
                Mix_GetError());
    Mix_AllocateChannels(16);

    for (int i = 0; i < SFX_COUNT; i++)
        a->sfx[i] = load_chunk(SFX_PATHS[i]);

    s_music = Mix_LoadMUS(MUSIC_PATH);
    if (!s_music)
        fprintf(stderr, "WARNING audio: could not load '%s': %s\n",
                MUSIC_PATH, Mix_GetError());

    fprintf(stderr,
        "INFO: audio — jump:%s stomp:%s death:%s spring:%s "
        "lootbox_spawn:%s lootbox_open:%s boss_spawn:%s music:%s\n",
        a->sfx[SFX_JUMP]         ? "OK" : "missing",
        a->sfx[SFX_STOMP]        ? "OK" : "missing",
        a->sfx[SFX_DEATH]        ? "OK" : "missing",
        a->sfx[SFX_SPRING]       ? "OK" : "missing",
        a->sfx[SFX_LOOTBOX_SPAWN]? "OK" : "missing",
        a->sfx[SFX_LOOTBOX_OPEN] ? "OK" : "missing",
        a->sfx[SFX_BOSS_SPAWN]   ? "OK" : "missing",
        s_music                  ? "OK" : "missing");
}

void audio_play_sfx(Audio *a, SfxId id) {
    if (id < 0 || id >= SFX_COUNT) return;
    if (!a->sfx[id]) return;
    Mix_PlayChannel(-1, a->sfx[id], 0);
}

void audio_play_music(Audio *a) {
    (void)a;
    if (!s_music) return;
    if (Mix_PlayMusic(s_music, -1) < 0)
        fprintf(stderr, "WARNING audio: Mix_PlayMusic: %s\n", Mix_GetError());
}

void audio_stop_music(void)  { Mix_HaltMusic(); }
void audio_pause_music(void) { if (Mix_PlayingMusic()) Mix_PauseMusic(); }
void audio_resume_music(void){ if (Mix_PausedMusic())  Mix_ResumeMusic(); }

void audio_set_sfx_volume(Audio *a, int vol) {
    for (int i = 0; i < SFX_COUNT; i++)
        if (a->sfx[i]) Mix_VolumeChunk(a->sfx[i], vol);
}
void audio_set_music_volume(int vol) { Mix_VolumeMusic(vol); }

void audio_free(Audio *a) {
    for (int i = 0; i < SFX_COUNT; i++)
        if (a->sfx[i]) Mix_FreeChunk(a->sfx[i]);
    if (s_music) { Mix_FreeMusic(s_music); s_music = NULL; }
    Mix_CloseAudio();
    Mix_Quit();
    memset(a, 0, sizeof(*a));
}

#else
void audio_load(Audio *a)               { memset(a,0,sizeof(*a));
                                          fprintf(stderr,"INFO: SDL2_mixer not compiled in\n"); }
void audio_play_sfx(Audio *a, SfxId id) { (void)a;(void)id; }
void audio_play_music(Audio *a)          { (void)a; }
void audio_stop_music(void)              {}
void audio_pause_music(void)             {}
void audio_resume_music(void)            {}
void audio_set_sfx_volume(Audio *a,int v){ (void)a;(void)v; }
void audio_set_music_volume(int v)       { (void)v; }
void audio_free(Audio *a)                { (void)a; }
#endif
