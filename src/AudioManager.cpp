#include "AudioManager.h"
#include <iostream>

// Dùng cerr thay vì cout để output không bị buffer (hiện ngay trong console)
#define LOG(msg) std::cerr << msg

// ──────────────────────────────────────────────────────────────────
// Biến SFX (static, chỉ dùng trong file này)
// ──────────────────────────────────────────────────────────────────
static MIX_Audio* rotateSfx     = nullptr;
static MIX_Audio* landSfx       = nullptr;
static MIX_Audio* clearShortSfx = nullptr;
static MIX_Audio* clearDoubleSfx = nullptr;
static MIX_Audio* clearTripleSfx = nullptr;
static MIX_Audio* clearLongSfx  = nullptr;
static MIX_Audio* gameOverSfx   = nullptr;
static MIX_Audio* buttonSfx     = nullptr;

// ──────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ──────────────────────────────────────────────────────────────────

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

// ──────────────────────────────────────────────────────────────────
// init()
// ──────────────────────────────────────────────────────────────────
bool AudioManager::init() {

    // 1) Init SDL_mixer
    if (!MIX_Init()) {
        LOG("[Audio] MIX_Init() FAIL: " << SDL_GetError() << "\n");
        return false;
    }

    // 2) Tạo mixer (tự mở audio device)
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer) {
        LOG("[Audio] MIX_CreateMixerDevice() FAIL: " << SDL_GetError() << "\n");
        return false;
    }

    // 3) Load file âm thanh
    // predecode=true → giải nén thành PCM lúc load
    bgm          = MIX_LoadAudio(mixer, "assets/sounds/nhacnen.wav",  true);
    moveSfx      = MIX_LoadAudio(mixer, "assets/sounds/move.wav",     true);
    rotateSfx    = MIX_LoadAudio(mixer, "assets/sounds/xoay.wav",     true);
    landSfx      = MIX_LoadAudio(mixer, "assets/sounds/land.wav",     true);
    clearShortSfx  = MIX_LoadAudio(mixer, "assets/sounds/clear1.wav", true);
    clearDoubleSfx = MIX_LoadAudio(mixer, "assets/sounds/clear2.wav", true);
    // clear3.wav có thể bị lỗi format → thử predecode=false rồi predecode=true
    clearTripleSfx = MIX_LoadAudio(mixer, "assets/sounds/clear3.wav", false);
    if (!clearTripleSfx)
        clearTripleSfx = MIX_LoadAudio(mixer, "assets/sounds/clear3.wav", true);
    clearLongSfx   = MIX_LoadAudio(mixer, "assets/sounds/clear4.wav", true);
    gameOverSfx  = MIX_LoadAudio(mixer, "assets/sounds/gameover.wav", true);
    buttonSfx    = MIX_LoadAudio(mixer, "assets/sounds/button.wav",   true);

    // Log kết quả (dùng cerr → hiện ngay, không bị buffer)
    LOG("[Audio] BGM:      " << (bgm            ? "OK" : "FAIL") << "\n");
    LOG("[Audio] move:     " << (moveSfx        ? "OK" : "FAIL") << "\n");
    LOG("[Audio] xoay:     " << (rotateSfx      ? "OK" : "FAIL") << "\n");
    LOG("[Audio] land:     " << (landSfx        ? "OK" : "FAIL") << "\n");
    LOG("[Audio] clear1:   " << (clearShortSfx  ? "OK" : "FAIL") << "\n");
    LOG("[Audio] clear2:   " << (clearDoubleSfx ? "OK" : "FAIL") << "\n");
    LOG("[Audio] clear3:   " << (clearTripleSfx ? "OK" : "FAIL") << "\n");
    LOG("[Audio] clear4:   " << (clearLongSfx   ? "OK" : "FAIL") << "\n");
    LOG("[Audio] gameover: " << (gameOverSfx    ? "OK" : "FAIL") << "\n");
    LOG("[Audio] button:   " << (buttonSfx      ? "OK" : "FAIL") << "\n");

    // CHỈ yêu cầu BGM bắt buộc; SFX thiếu thì bỏ qua, không fail toàn bộ.
    // File clear3.wav có thể bị hỏng format → nếu fail thì dùng clear1 thay thế.
    if (!bgm) {
        LOG("[Audio] BGM load FAIL - không có nhạc nền!\n");
        return false;
    }

    // Fallback: nếu clear3 load fail → dùng lại clear1
    if (!clearTripleSfx && clearShortSfx) {
        LOG("[Audio] clear3.wav lỗi → fallback sang clear1.wav\n");
        clearTripleSfx = clearShortSfx;
    }

    // 4) Tạo track BGM
    bgmTrack = MIX_CreateTrack(mixer);
    if (!bgmTrack) {
        LOG("[Audio] CreateTrack BGM FAIL\n");
        return false;
    }
    MIX_SetTrackAudio(bgmTrack, bgm);

    // 5) Tạo track SFX (tái dùng 1 track duy nhất thay vì fire-and-forget)
    // Lý do: MIX_PlayAudio fire-and-forget tạo track tạm nội bộ mỗi lần gọi.
    // Khi bấm Z/X liên tục nhanh, quá nhiều track tạm đồng thời có thể gây
    // lag hoặc lỗi. Dùng 1 sfxTrack cố định + StopTrack trước khi play mới
    // đảm bảo ổn định.
    sfxTrack = MIX_CreateTrack(mixer);
    if (!sfxTrack) {
        LOG("[Audio] CreateTrack SFX FAIL\n");
        // Không return false — game vẫn chạy được, chỉ mất SFX
    }

    initialized = true;
    LOG("[Audio] Init thành công!\n");
    return true;
}

// ──────────────────────────────────────────────────────────────────
// shutdown()
// ──────────────────────────────────────────────────────────────────
void AudioManager::shutdown() {
    if (bgmTrack) { MIX_DestroyTrack(bgmTrack); bgmTrack = nullptr; }
    if (sfxTrack) { MIX_DestroyTrack(sfxTrack); sfxTrack = nullptr; }
    if (mixer)    { MIX_DestroyMixer(mixer);    mixer    = nullptr; }

    // Hủy audio buffers
    // Lưu ý: clearTripleSfx có thể == clearShortSfx (fallback), không hủy 2 lần
    auto freeAudio = [](MIX_Audio*& a) {
        if (a) { MIX_DestroyAudio(a); a = nullptr; }
    };
    freeAudio(bgm);
    freeAudio(moveSfx);
    freeAudio(rotateSfx);
    freeAudio(landSfx);
    // Nếu clearTripleSfx là con trỏ fallback thì đặt null trước để tránh double-free
    if (clearTripleSfx == clearShortSfx) clearTripleSfx = nullptr;
    freeAudio(clearShortSfx);
    freeAudio(clearDoubleSfx);
    freeAudio(clearTripleSfx);
    freeAudio(clearLongSfx);
    freeAudio(gameOverSfx);
    freeAudio(buttonSfx);

    MIX_Quit();
    initialized = false;
}

// ──────────────────────────────────────────────────────────────────
// BGM
// ──────────────────────────────────────────────────────────────────
void AudioManager::playBGM() {
    if (!bgmEnabled || !bgmTrack) return;

    // SDL3_mixer 3.2: MIX_PlayTrack(track, SDL_PropertiesID)
    // Loop vô hạn = set MIX_PROP_PLAY_LOOPS_NUMBER = -1
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

    bool ok = MIX_PlayTrack(bgmTrack, props);
    LOG("[Audio] playBGM: " << (ok ? "OK" : "FAIL") << "\n");
    if (!ok) {
        LOG("[Audio] playBGM error: " << SDL_GetError() << "\n");
    }

    SDL_DestroyProperties(props);
}

void AudioManager::pauseBGM() {
    if (bgmTrack) MIX_PauseTrack(bgmTrack);
}

void AudioManager::resumeBGM() {
    if (bgmTrack) MIX_ResumeTrack(bgmTrack);
}

void AudioManager::stopBGM() {
    if (bgmTrack) MIX_StopTrack(bgmTrack, 0);
}

void AudioManager::setBGMVolume(float volume) {
    if (bgmTrack) MIX_SetTrackGain(bgmTrack, volume);
}

// ──────────────────────────────────────────────────────────────────
// SFX
//
// Dùng sfxTrack tái dùng: stop cũ → gán audio mới → play.
// Đảm bảo không tạo quá nhiều track tạm khi bấm phím liên tục.
// ──────────────────────────────────────────────────────────────────
void AudioManager::playSFX(SoundType sound) {
    if (!sfxEnabled || !mixer) return;

    MIX_Audio* audio = nullptr;

    switch (sound) {
        case SoundType::MOVE:          audio = moveSfx;        break;
        case SoundType::ROTATE:        audio = rotateSfx;      break;
        case SoundType::LOCK:          audio = landSfx;        break;
        case SoundType::CLEAR_SINGLE:  audio = clearShortSfx;  break;
        case SoundType::CLEAR_DOUBLE:  audio = clearDoubleSfx; break;
        case SoundType::CLEAR_TRIPLE:  audio = clearTripleSfx; break;
        case SoundType::CLEAR_TETRIS:  audio = clearLongSfx;   break;
        case SoundType::GAME_OVER:     audio = gameOverSfx;    break;
        case SoundType::BUTTON:        audio = buttonSfx;      break;
    }

    if (!audio) return;

    if (sfxTrack) {
        // Dừng SFX đang phát (nếu có), gán audio mới, phát
        MIX_StopTrack(sfxTrack, 0);
        MIX_SetTrackAudio(sfxTrack, audio);
        MIX_PlayTrack(sfxTrack, 0); // 0 = default props = không loop
    } else {
        // Fallback: nếu sfxTrack bị null → dùng fire-and-forget
        MIX_PlayAudio(mixer, audio);
    }
}

void AudioManager::setSFXVolume(float volume) {
    if (sfxTrack) MIX_SetTrackGain(sfxTrack, volume);
}

// ──────────────────────────────────────────────────────────────────
// Settings
// ──────────────────────────────────────────────────────────────────

void AudioManager::toggleBGM()  { setBGMEnabled(!bgmEnabled); }
void AudioManager::toggleSFX()  { sfxEnabled = !sfxEnabled; }

void AudioManager::setBGMEnabled(bool enabled) {
    bgmEnabled = enabled;
    if (!enabled) pauseBGM();
    else          resumeBGM();
}

void AudioManager::setSFXEnabled(bool enabled) { sfxEnabled = enabled; }
bool AudioManager::isBGMEnabled() const { return bgmEnabled; }
bool AudioManager::isSFXEnabled() const { return sfxEnabled; }

// ──────────────────────────────────────────────────────────────────
// handleEvent()
// ──────────────────────────────────────────────────────────────────
void AudioManager::handleEvent(const SDL_Event& e) {
    if (!sfxEnabled) return;
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e.button.button == SDL_BUTTON_LEFT) {
        playSFX(SoundType::BUTTON);
    }
}

// ──────────────────────────────────────────────────────────────────
// printSettings()
// ──────────────────────────────────────────────────────────────────
void AudioManager::printSettings() {
    LOG("\n===== AUDIO SETTINGS =====\n");
    LOG("Music: " << (bgmEnabled ? "ON" : "OFF") << "\n");
    LOG("SFX:   " << (sfxEnabled ? "ON" : "OFF") << "\n");
}