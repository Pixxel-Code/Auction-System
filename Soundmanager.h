#pragma once
#include <windows.h>
#include <mmsystem.h>

// Requires: winmm.lib in Linker -> Additional Dependencies

inline void playSound(const char* filename) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* last = strrchr(path, '\\');
    if (last) {
        *(last + 1) = '\0';
        strcat_s(path, MAX_PATH, filename);
        PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
}

inline void playBidDing() { playSound("bid_ding.wav"); }
inline void playClick() { playSound("btn_click.wav"); }
inline void playNotification() { playSound("notification.wav"); }