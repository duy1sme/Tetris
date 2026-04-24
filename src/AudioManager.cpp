#include "AudioManager.h"

AudioManager::AudioManager()
    : initialized(false)
    , bgmEnabled(true)
    , sfxEnabled(true) {
}

AudioManager::~AudioManager() { }

bool AudioManager::init()         { return true; }
void AudioManager::shutdown()     { }
void AudioManager::playBGM()      { }
void AudioManager::stopBGM()      { }
void AudioManager::pauseBGM()     { }
void AudioManager::resumeBGM()    { }
void AudioManager::setBGMVolume(int volume) { }
void AudioManager::playSFX(SoundType sound) { }
void AudioManager::setSFXVolume(int volume) { }
void AudioManager::toggleBGM()    { bgmEnabled = !bgmEnabled; }
void AudioManager::toggleSFX()    { sfxEnabled = !sfxEnabled; }
bool AudioManager::isBGMEnabled() const { return bgmEnabled; }
bool AudioManager::isSFXEnabled() const { return sfxEnabled; }