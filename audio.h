#ifndef AUDIO_H
#define AUDIO_H

/*
 * Audio module — wraps SDL2_mixer for SFX and music.
 * Safe to call regardless of whether SDL2_MIXER_FOUND is defined.
 *
 * Expected assets:
 *   assets/music.ogg        — looping background track
 *   assets/jump.wav         — player jump
 *   assets/stomp.wav        — enemy stomped
 *   assets/death.wav        — player killed
 *   assets/spring.wav       — player/enemy hits spring tile
 *   assets/lootbox_spawn.wav — loot box appears
 *   assets/lootbox_open.wav  — loot box opened
 *   assets/boss_spawn.wav   — boss enemy appears
 */

typedef enum {
    SFX_JUMP         = 0,
    SFX_STOMP        = 1,
    SFX_DEATH        = 2,
    SFX_SPRING       = 3,
    SFX_LOOTBOX_SPAWN= 4,
    SFX_LOOTBOX_OPEN = 5,
    SFX_BOSS_SPAWN   = 6,
    SFX_COUNT        = 7
} SfxId;

typedef struct {
#ifdef SDL2_MIXER_FOUND
    struct Mix_Chunk *sfx[SFX_COUNT];
#else
    int _dummy;
#endif
} Audio;

void audio_load(Audio *a);
void audio_play_sfx(Audio *a, SfxId id);
void audio_play_music(Audio *a);
void audio_stop_music(void);
void audio_pause_music(void);
void audio_resume_music(void);
void audio_set_sfx_volume(Audio *a, int vol);
void audio_set_music_volume(int vol);
void audio_free(Audio *a);

#endif
