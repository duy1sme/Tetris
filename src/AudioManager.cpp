#include "AudioManager.h"
#include <iostream>

// Bo dem am thanh chi dung trong file nay, khong dua ra header.
static MIX_Audio* rotateSfx     = nullptr;
static MIX_Audio* landSfx       = nullptr;
static MIX_Audio* clearShortSfx = nullptr;
static MIX_Audio* clearLongSfx  = nullptr;
static MIX_Audio* gameOverSfx   = nullptr;
static MIX_Audio* buttonSfx     = nullptr;

namespace {
MIX_Audio* loadAudioWithFallback(MIX_Mixer* mixer, const char* primaryPath, const char* secondaryPath) {
    MIX_Audio* audio = MIX_LoadAudio_IO(mixer, SDL_IOFromFile(primaryPath, "rb"), true, true);
    if (!audio && secondaryPath) {
        audio = MIX_LoadAudio_IO(mixer, SDL_IOFromFile(secondaryPath, "rb"), true, true);
    }
    return audio;
}

void destroyAudio(MIX_Audio*& audio) {
    if (audio) {
        MIX_DestroyAudio(audio);
        audio = nullptr;
    }
}
} // namespace

AudioManager::AudioManager()
    : initialized(false),
      bgmEnabled(true),
      sfxEnabled(true),
      mixer(nullptr),
      bgmTrack(nullptr),
      sfxTrack(nullptr),
      bgm(nullptr),
      moveSfx(nullptr) {}

AudioManager::~AudioManager() {
    shutdown();
}

//
// ======================================================
// 🚀 INIT AUDIO SYSTEM
// ======================================================
// Khởi tạo toàn bộ hệ thống âm thanh:
// 1) SDL audio subsystem
// 2) SDL_mixer
// 3) Audio device + mixer
// 4) Tai tat ca file .wav
// 5) Tao kenh phat
//

bool AudioManager::init() {

    // 1) Chỉ bật subsystem audio (an toàn khi SDL đã init ở Game).
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // 2) Khoi tao mixer.
    if (MIX_Init() < 0) {
        std::cout << "MIX init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // 3) Cau hinh dinh dang phat.
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;

    SDL_AudioDeviceID device =
        SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

    if (device == 0) {
        std::cout << "Open audio device failed: " << SDL_GetError() << "\n";
        return false;
    }
    SDL_ResumeAudioDevice(device);

    // 4) Gan mixer vao thiet bi audio.
    mixer = MIX_CreateMixerDevice(device, &spec);
    if (!mixer) {
        std::cout << "Create mixer failed: " << SDL_GetError() << "\n";
        return false;
    }

    //
    // =========================
    // Tai tep am thanh.
    // =========================
    // Lưu ý quan trọng:
    // - Path "assets/xxx.wav" là relative path
    // - Khi chạy .exe, working dir phải chứa thư mục assets/
    // - Neu dong goi sai thi tai se that bai
    //

    // Hỗ trợ cả cấu trúc cũ `assets/sounds/*` và cấu trúc mới `assets/sounds/sounds/*`.
    bgm           = loadAudioWithFallback(mixer, "assets/sounds/nhacnen.wav", "assets/sounds/sounds/nhacnen.wav");
    moveSfx       = loadAudioWithFallback(mixer, "assets/sounds/move.wav", "assets/sounds/sounds/move.wav");
    rotateSfx     = loadAudioWithFallback(mixer, "assets/sounds/xoay.wav", "assets/sounds/sounds/xoay.wav");
    landSfx       = loadAudioWithFallback(mixer, "assets/sounds/land.wav", "assets/sounds/sounds/land.wav");
    clearShortSfx = loadAudioWithFallback(mixer, "assets/sounds/clearshort.wav", "assets/sounds/sounds/clearshort.wav");
    clearLongSfx  = loadAudioWithFallback(mixer, "assets/sounds/clearlong.wav", "assets/sounds/sounds/clearlong.wav");
    gameOverSfx   = loadAudioWithFallback(mixer, "assets/sounds/gameover.wav", "assets/sounds/sounds/gameover.wav");
    buttonSfx     = loadAudioWithFallback(mixer, "assets/sounds/button.wav", "assets/sounds/sounds/button.wav");

    // Neu thieu bat ky file bat buoc nao thi dung som.
    if (!bgm || !moveSfx || !rotateSfx || !landSfx ||
        !clearShortSfx || !clearLongSfx || !gameOverSfx || !buttonSfx) {
        std::cout << "Load audio failed (check assets path!)\n";
        return false;
    }

    //
    // =========================
    // Tao kenh phat.
    // =========================
    bgmTrack = MIX_CreateTrack(mixer);
    sfxTrack = MIX_CreateTrack(mixer);

    if (!bgmTrack || !sfxTrack) {
        std::cout << "Create track failed\n";
        return false;
    }

    MIX_SetTrackAudio(bgmTrack, bgm);

    initialized = true;
    return true;
}

//
// ======================================================
// 🧹 GIẢI PHÓNG
// ======================================================
// Don dep toan bo tai nguyen (kenh phat, mixer, SDL)
//

void AudioManager::shutdown() {

    if (bgmTrack) {
        MIX_DestroyTrack(bgmTrack);
        bgmTrack = nullptr;
    }

    if (sfxTrack) {
        MIX_DestroyTrack(sfxTrack);
        sfxTrack = nullptr;
    }

    if (mixer) {
        MIX_DestroyMixer(mixer);
        mixer = nullptr;
    }

    // Giai phong tat ca du lieu am thanh da tai.
    destroyAudio(bgm);
    destroyAudio(moveSfx);
    destroyAudio(rotateSfx);
    destroyAudio(landSfx);
    destroyAudio(clearShortSfx);
    destroyAudio(clearLongSfx);
    destroyAudio(gameOverSfx);
    destroyAudio(buttonSfx);

    // Tat mixer library va audio subsystem cua SDL.
    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    initialized = false;
}

//
// ======================================================
// 🎵 BGM (NHẠC NỀN)
// ======================================================
//

void AudioManager::playBGM() {
    // -1 = lap vo han.
    if (bgmEnabled && bgmTrack)
        MIX_PlayTrack(bgmTrack, -1);
}

void AudioManager::pauseBGM() {
    if (bgmTrack)
        MIX_PauseTrack(bgmTrack);
}

void AudioManager::resumeBGM() {
    if (bgmTrack)
        MIX_ResumeTrack(bgmTrack);
}

void AudioManager::stopBGM() {
    // Dung ngay lap tuc (khong fade out).
    if (bgmTrack)
        MIX_StopTrack(bgmTrack, 0);
}

void AudioManager::setBGMVolume(float volume) {
    // Volume nam trong [0.0, 1.0].
    if (bgmTrack)
        MIX_SetTrackGain(bgmTrack, volume);
}

//
// ======================================================
// 🔊 SFX (HIỆU ỨNG)
// ======================================================
void AudioManager::playSFX(SoundType sound) {

    if (!sfxEnabled || !mixer) return;

    MIX_Audio* audio = nullptr;

    // Anh xa loai su kien sang doan am thanh da tai.
    switch (sound) {
        case SoundType::MOVE:          audio = moveSfx; break;
        case SoundType::ROTATE:        audio = rotateSfx; break;
        case SoundType::LOCK:          audio = landSfx; break;
        case SoundType::CLEAR_SINGLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_DOUBLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_TRIPLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_TETRIS:  audio = clearLongSfx; break;
        case SoundType::GAME_OVER:     audio = gameOverSfx; break;
        case SoundType::BUTTON:        audio = buttonSfx; break;
    }

    if (audio && sfxTrack) {
        // Tai su dung mot kenh SFX de tranh cap phat moi moi lan phat.
        MIX_SetTrackAudio(sfxTrack, audio);
        MIX_PlayTrack(sfxTrack, 0);
    }
}

void AudioManager::setSFXVolume(float volume) {
    if (sfxTrack)
        MIX_SetTrackGain(sfxTrack, volume);
}

//
// ======================================================
// ⚙️ SETTINGS (ON / OFF)
// ======================================================
//

void AudioManager::toggleBGM() {
    setBGMEnabled(!bgmEnabled);
}

void AudioManager::toggleSFX() {
    sfxEnabled = !sfxEnabled;
}

void AudioManager::setBGMEnabled(bool enabled) {
    bgmEnabled = enabled;

    // Neu tat thi tam dung, bat lai thi tiep tuc.
    if (!enabled) pauseBGM();
    else resumeBGM();
}

void AudioManager::setSFXEnabled(bool enabled) {
    sfxEnabled = enabled;
}

bool AudioManager::isBGMEnabled() const {
    return bgmEnabled;
}

bool AudioManager::isSFXEnabled() const {
    return sfxEnabled;
}

//
// ======================================================
// In trang thai cai dat de debug.
// ======================================================
//

void AudioManager::printSettings() {
    std::cout << "\n===== SETTINGS =====\n";
    std::cout << "Music: " << (bgmEnabled ? "ON" : "OFF") << "\n";
    std::cout << "SFX:   " << (sfxEnabled ? "ON" : "OFF") << "\n";
}

//
// ======================================================
// 🖱 HANDLE EVENT (CLICK → SOUND)
// ======================================================
// Tách riêng xử lý click để Game.cpp chỉ cần gọi 1 dòng
//

void AudioManager::handleEvent(const SDL_Event& e) {

    if (!sfxEnabled) return;

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {

            std::cout << "CLICK DETECTED\n";

            playSFX(SoundType::BUTTON);
        }
    }
}