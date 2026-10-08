#include "audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

static char s_audio_dir[MAX_PATH] = "assets/audio";
static bool s_sound_enabled = true;
static bool s_music_playing = false;

void audio_init(const char *audio_dir) {
    if (audio_dir && strlen(audio_dir) > 0) {
        strncpy(s_audio_dir, audio_dir, sizeof(s_audio_dir) - 1);
        s_audio_dir[sizeof(s_audio_dir) - 1] = '\0';
    }
}

void audio_play_sfx(SoundEffect sfx) {
    if (!s_sound_enabled) return;

    const char *filename = NULL;
    switch (sfx) {
        case SFX_MOVE:          filename = "CLICK1.WAV"; break;
        case SFX_PICKUP_CHIP:   filename = "CLICK3.WAV"; break;
        case SFX_PICKUP_KEY:
        case SFX_PICKUP_BOOTS:  filename = "BLIP2.WAV"; break;
        case SFX_OPEN_DOOR:     filename = "DOOR.WAV"; break;
        case SFX_SOCKET_OPEN:   filename = "CHIMES.WAV"; break;
        case SFX_WATER:         filename = "WATER2.WAV"; break;
        case SFX_BUMP:          filename = "OOF3.WAV"; break;
        case SFX_DEATH:         filename = "BUMMER.WAV"; break;
        case SFX_WIN:           filename = "DITTY1.WAV"; break;
        case SFX_TELEPORT:      filename = "TELEPORT.WAV"; break;
        case SFX_BOMB:          filename = "POP2.WAV"; break;
        default: break;
    }

    if (filename) {
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", s_audio_dir, filename);
        PlaySoundA(full_path, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
}

void audio_play_music(int music_index) {
    if (!s_sound_enabled) return;

    audio_stop_music();

    char mid_file[MAX_PATH];
    snprintf(mid_file, sizeof(mid_file), "%s/CHIP%02d.MID", s_audio_dir, music_index == 2 ? 2 : 1);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "open \"%s\" type sequencer alias cc_bgm", mid_file);
    if (mciSendStringA(cmd, NULL, 0, NULL) == 0) {
        mciSendStringA("play cc_bgm repeat", NULL, 0, NULL);
        s_music_playing = true;
    }
}

void audio_stop_music(void) {
    if (s_music_playing) {
        mciSendStringA("stop cc_bgm", NULL, 0, NULL);
        mciSendStringA("close cc_bgm", NULL, 0, NULL);
        s_music_playing = false;
    }
}

void audio_toggle_sound(void) {
    s_sound_enabled = !s_sound_enabled;
    if (!s_sound_enabled) {
        audio_stop_music();
    } else {
        audio_play_music(1);
    }
}

void audio_cleanup(void) {
    audio_stop_music();
}
