#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "GameState.h"

class AudioManager {
private:
    bool initialized;
    bool bgmEnabled;
    bool sfxEnabled;

    MIX_Mixer* mixer;
    MIX_Track* bgmTrack; // Kenh BGM duoc dung xuyen suot.
    MIX_Track* sfxTrack; // Kenh SFX dung chung.

    MIX_Audio* bgm;
    MIX_Audio* moveSfx;

public:
    AudioManager();
    ~AudioManager();

    // Vong doi AudioManager.
    bool init();
    void shutdown();

    // Dieu khien BGM.
    void playBGM();
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    void setBGMVolume(float volume); // [0.0..1.0]

    // Dieu khien SFX.
    void playSFX(SoundType sound);
    void setSFXVolume(float volume);

    // Cai dat bat/tat.
    void toggleBGM();
    void toggleSFX();
    bool isBGMEnabled() const;
    bool isSFXEnabled() const;
    void setBGMEnabled(bool enabled);
    void setSFXEnabled(bool enabled);

    // Xu ly event de phat am click trong giao dien.
    void handleEvent(const SDL_Event& e);

    // Ham ho tro debug.
    void printSettings();
};

#endif