#ifndef CHIPS_AUDIO_H
#define CHIPS_AUDIO_H

#include <stdbool.h>

typedef enum SoundEffect {
    SFX_MOVE,
    SFX_PICKUP_CHIP,
    SFX_PICKUP_KEY,
    SFX_PICKUP_BOOTS,
    SFX_OPEN_DOOR,
    SFX_SOCKET_OPEN,
    SFX_WATER,
    SFX_BUMP,
    SFX_DEATH,
    SFX_WIN,
    SFX_TELEPORT,
    SFX_BOMB
} SoundEffect;

void audio_init(const char *audio_dir);
void audio_play_sfx(SoundEffect sfx);
void audio_play_music(int music_index); /* 1 for CHIP01.MID, 2 for CHIP02.MID */
void audio_stop_music(void);
void audio_toggle_sound(void);
bool audio_is_enabled(void);
void audio_cleanup(void);

#endif /* CHIPS_AUDIO_H */
