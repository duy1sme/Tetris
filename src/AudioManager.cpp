/*
 * AudioManager.cpp
 * ----------------
 * Triển khai AudioManager — khởi tạo SDL3_mixer, load file âm thanh
 * và điều khiển BGM/SFX trong suốt ván chơi.
 *
 * Kiến trúc kênh phát:
 *  - bgmTrack : một kênh duy nhất dành riêng cho nhạc nền.
 *               Audio được gán vào track một lần duy nhất lúc init(),
 *               sau đó chỉ cần Play/Pause/Resume/Stop.
 *  - sfxTrack : một kênh dùng chung cho tất cả SFX.
 *               Trước mỗi lần phát, gán audio mới vào track (tái dùng
 *               kênh, không cấp phát track mới mỗi lần).
 *
 * FIX nhạc nền dừng sau ~2 phút:
 *   Nguyên nhân: MIX_PlayTrack(bgmTrack, -1) bị SDL3_mixer hiểu là
 *   "phát 0 lần lặp" (tham số loopCount = -1 không có nghĩa vô hạn
 *   trong SDL3_mixer API mới).
 *   Sửa: dùng MIX_INFINITY thay vì -1 để phát lặp vô tận.
 */

#include "AudioManager.h"
#include <iostream>

// ============================================================
// Dữ liệu âm thanh SFX — static, chỉ dùng trong file này.
// Khai báo ở cấp file thay vì trong header để tránh lộ ra ngoài.
// ============================================================
static MIX_Audio* rotateSfx     = nullptr;
static MIX_Audio* landSfx       = nullptr;
static MIX_Audio* clearShortSfx = nullptr;
static MIX_Audio* clearLongSfx  = nullptr;
static MIX_Audio* gameOverSfx   = nullptr;
static MIX_Audio* buttonSfx     = nullptr;

// ============================================================
// Hàm tiện ích nội bộ (ẩn trong anonymous namespace)
// ============================================================
namespace {

// Load audio từ đường dẫn chính; nếu thất bại thử đường dẫn phụ.
// Hỗ trợ cả cấu trúc thư mục cũ (assets/sounds/) và mới
// (assets/sounds/sounds/) để tương thích nhiều môi trường build.
MIX_Audio* loadAudioWithFallback(MIX_Mixer* mixer,
                                  const char* primaryPath,
                                  const char* secondaryPath) {
    MIX_Audio* audio = MIX_LoadAudio_IO(mixer,
                           SDL_IOFromFile(primaryPath, "rb"), true, true);
    if (!audio && secondaryPath) {
        audio = MIX_LoadAudio_IO(mixer,
                    SDL_IOFromFile(secondaryPath, "rb"), true, true);
    }
    return audio;
}

// Hủy audio buffer an toàn (null-check + đặt về nullptr sau khi hủy).
void destroyAudio(MIX_Audio*& audio) {
    if (audio) {
        MIX_DestroyAudio(audio);
        audio = nullptr;
    }
}

} // namespace

// ============================================================
// Constructor / Destructor
// ============================================================

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

// ============================================================
// 🚀 INIT — Khởi tạo toàn bộ hệ thống âm thanh
// ============================================================
// Thứ tự khởi tạo phải đúng:
//   1) SDL audio subsystem  →  2) MIX_Init
//   →  3) Mở audio device   →  4) Tạo Mixer
//   →  5) Load file .wav    →  6) Tạo Track (kênh phát)
//
// Nếu bất kỳ bước nào thất bại, hàm trả về false ngay lập tức.
// Game vẫn chạy được nhưng không có âm thanh.
// ============================================================

bool AudioManager::init() {

    // Bước 1: Bật audio subsystem (an toàn khi SDL đã init ở Game::init()).
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL audio init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Bước 2: Khởi tạo thư viện SDL_mixer.
    if (MIX_Init() < 0) {
        std::cout << "MIX_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Bước 3: Cấu hình định dạng phát audio.
    //   freq=44100 Hz (CD quality), format=Float32, channels=2 (stereo).
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq     = 44100;
    spec.format   = SDL_AUDIO_F32;
    spec.channels = 2;

    // Mở thiết bị audio mặc định của hệ thống.
    SDL_AudioDeviceID device =
        SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (device == 0) {
        std::cout << "Open audio device failed: " << SDL_GetError() << "\n";
        return false;
    }
    // SDL3 mở device ở trạng thái paused → cần resume thủ công.
    SDL_ResumeAudioDevice(device);

    // Bước 4: Gắn Mixer vào thiết bị audio.
    mixer = MIX_CreateMixerDevice(device, &spec);
    if (!mixer) {
        std::cout << "Create mixer failed: " << SDL_GetError() << "\n";
        return false;
    }

    // ── Bước 5: Load file âm thanh ────────────────────────────────────
    // Lưu ý: đường dẫn là relative từ thư mục chứa file .exe.
    //   Khi chạy từ IDE hay command line, cần đảm bảo working directory
    //   chứa thư mục assets/.
    bgm           = loadAudioWithFallback(mixer,
                        "assets/sounds/nhacnen.wav");
    moveSfx       = loadAudioWithFallback(mixer,
                        "assets/sounds/move.wav");
    rotateSfx     = loadAudioWithFallback(mixer,
                        "assets/sounds/xoay.wav");
    landSfx       = loadAudioWithFallback(mixer,
                        "assets/sounds/land.wav");
    clearShortSfx = loadAudioWithFallback(mixer,
                        "assets/sounds/clearshort.wav");
    clearLongSfx  = loadAudioWithFallback(mixer,
                        "assets/sounds/clearlong.wav");
    gameOverSfx   = loadAudioWithFallback(mixer,
                        "assets/sounds/gameover.wav");
    buttonSfx     = loadAudioWithFallback(mixer,
                        "assets/sounds/button.wav");

    // Nếu thiếu bất kỳ file bắt buộc nào → dừng khởi tạo sớm.
    if (!bgm || !moveSfx || !rotateSfx || !landSfx ||
        !clearShortSfx || !clearLongSfx || !gameOverSfx || !buttonSfx) {
        std::cout << "Load audio failed — kiểm tra thư mục assets/sounds/\n";
        return false;
    }

    // ── Bước 6: Tạo kênh phát ─────────────────────────────────────────
    bgmTrack = MIX_CreateTrack(mixer); // Kênh riêng cho nhạc nền.
    sfxTrack = MIX_CreateTrack(mixer); // Kênh dùng chung cho SFX.

    if (!bgmTrack || !sfxTrack) {
        std::cout << "Create track failed\n";
        return false;
    }

    // Gán audio nhạc nền vào bgmTrack ngay từ đầu.
    // Audio này không thay đổi trong suốt game — chỉ Play/Pause/Resume.
    MIX_SetTrackAudio(bgmTrack, bgm);

    initialized = true;
    return true;
}

// ============================================================
// 🧹 SHUTDOWN — Giải phóng tài nguyên
// ============================================================
// Thứ tự hủy ngược với thứ tự khởi tạo:
//   Track → Mixer → Audio buffer → MIX_Quit → SDL subsystem

void AudioManager::shutdown() {

    // Hủy các kênh phát trước khi hủy mixer.
    if (bgmTrack) { MIX_DestroyTrack(bgmTrack); bgmTrack = nullptr; }
    if (sfxTrack) { MIX_DestroyTrack(sfxTrack); sfxTrack = nullptr; }

    // Hủy mixer (giải phóng kết nối với audio device).
    if (mixer) { MIX_DestroyMixer(mixer); mixer = nullptr; }

    // Giải phóng toàn bộ dữ liệu âm thanh đã load.
    destroyAudio(bgm);
    destroyAudio(moveSfx);
    destroyAudio(rotateSfx);
    destroyAudio(landSfx);
    destroyAudio(clearShortSfx);
    destroyAudio(clearLongSfx);
    destroyAudio(gameOverSfx);
    destroyAudio(buttonSfx);

    // Tắt thư viện SDL_mixer và audio subsystem.
    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    initialized = false;
}

// ============================================================
// 🎵 BGM — Nhạc nền
// ============================================================

// ✅ FIX: Dùng MIX_INFINITY thay vì -1 để lặp vô hạn.
//
// Nguyên nhân lỗi cũ: MIX_PlayTrack(bgmTrack, -1) — trong SDL3_mixer
// tham số thứ 2 là "số lần lặp thêm sau lần đầu" (additional loops),
// không phải sentinel -1 = vô hạn như SDL2_mixer. Giá trị -1 bị hiểu
// là 0 hoặc âm tràn số, dẫn đến nhạc phát hết file rồi dừng.
//
// MIX_INFINITY (= SDL_MAX_SINT32) là hằng số chính thức của SDL3_mixer
// để chỉ "lặp vô hạn".
void AudioManager::playBGM() {
    if (bgmEnabled && bgmTrack)
        MIX_PlayTrack(bgmTrack, MIX_INFINITY); // Lặp vô hạn — đúng API SDL3_mixer
}

void AudioManager::pauseBGM() {
    if (bgmTrack)
        MIX_PauseTrack(bgmTrack);
}

void AudioManager::resumeBGM() {
    // Tiếp tục từ vị trí đã dừng (không phát lại từ đầu).
    if (bgmTrack)
        MIX_ResumeTrack(bgmTrack);
}

void AudioManager::stopBGM() {
    // Dừng ngay lập tức — fadeOutMs = 0 (không fade out).
    if (bgmTrack)
        MIX_StopTrack(bgmTrack, 0);
}

void AudioManager::setBGMVolume(float volume) {
    // Gain: 1.0 = 100% (không khuếch đại), 0.0 = tắt tiếng.
    if (bgmTrack)
        MIX_SetTrackGain(bgmTrack, volume);
}

// ============================================================
// 🔊 SFX — Hiệu ứng âm thanh
// ============================================================

void AudioManager::playSFX(SoundType sound) {
    if (!sfxEnabled || !mixer) return;

    MIX_Audio* audio = nullptr;

    // Ánh xạ loại sự kiện sang dữ liệu âm thanh tương ứng.
    // CLEAR_SINGLE/DOUBLE/TRIPLE dùng chung clearShortSfx để đơn giản hóa.
    switch (sound) {
        case SoundType::MOVE:          audio = moveSfx;       break;
        case SoundType::ROTATE:        audio = rotateSfx;     break;
        case SoundType::LOCK:          audio = landSfx;       break;
        case SoundType::CLEAR_SINGLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_DOUBLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_TRIPLE:  audio = clearShortSfx; break;
        case SoundType::CLEAR_TETRIS:  audio = clearLongSfx;  break;
        case SoundType::GAME_OVER:     audio = gameOverSfx;   break;
        case SoundType::BUTTON:        audio = buttonSfx;     break;
    }

    if (audio && sfxTrack) {
        // Gán audio mới vào kênh SFX (tái dùng kênh thay vì tạo track mới).
        MIX_SetTrackAudio(sfxTrack, audio);
        MIX_PlayTrack(sfxTrack, 0); // 0 = phát 1 lần, không lặp.
    }
}

void AudioManager::setSFXVolume(float volume) {
    if (sfxTrack)
        MIX_SetTrackGain(sfxTrack, volume);
}

// ============================================================
// ⚙️ SETTINGS — Bật/tắt BGM và SFX
// ============================================================

void AudioManager::toggleBGM() {
    setBGMEnabled(!bgmEnabled);
}

void AudioManager::toggleSFX() {
    sfxEnabled = !sfxEnabled;
}

// Tắt BGM → pause (giữ vị trí phát), bật lại → resume (tiếp tục).
void AudioManager::setBGMEnabled(bool enabled) {
    bgmEnabled = enabled;
    if (!enabled) pauseBGM();
    else          resumeBGM();
}

void AudioManager::setSFXEnabled(bool enabled) {
    sfxEnabled = enabled;
}

bool AudioManager::isBGMEnabled() const { return bgmEnabled; }
bool AudioManager::isSFXEnabled() const { return sfxEnabled; }

// ============================================================
// 🖱 HANDLE EVENT — Click chuột → phát tiếng button
// ============================================================
// Tách riêng xử lý event để Game.cpp chỉ cần gọi 1 dòng:
//   audio.handleEvent(event);
// thay vì phải tự kiểm tra loại event và gọi playSFX.

void AudioManager::handleEvent(const SDL_Event& e) {
    if (!sfxEnabled) return;

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            std::cout << "CLICK DETECTED\n";
            playSFX(SoundType::BUTTON);
        }
    }
}

// ============================================================
// 🐛 DEBUG — In trạng thái cài đặt ra console
// ============================================================

void AudioManager::printSettings() {
    std::cout << "\n===== AUDIO SETTINGS =====\n";
    std::cout << "Music: " << (bgmEnabled ? "ON" : "OFF") << "\n";
    std::cout << "SFX:   " << (sfxEnabled ? "ON" : "OFF") << "\n";
}