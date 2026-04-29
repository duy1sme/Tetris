/*
 * AudioManager.h
 * --------------
 * Quản lý toàn bộ hệ thống âm thanh của game dùng SDL3_mixer.
 *
 * Kiến trúc:
 *  - Một MIX_Mixer duy nhất gắn với thiết bị audio đang dùng.
 *  - bgmTrack : kênh dành riêng cho nhạc nền (BGM), phát lặp vô hạn.
 *  - sfxTrack : kênh dùng chung cho tất cả hiệu ứng âm thanh (SFX).
 *               Mỗi khi cần phát SFX, audio mới được gán vào kênh này
 *               (tái dùng kênh, tránh cấp phát track mới mỗi lần).
 *
 * Vòng đời điển hình:
 *   AudioManager audio;
 *   audio.init();       ← gọi một lần sau khi SDL_Init
 *   audio.playBGM();    ← bắt đầu nhạc nền lặp vô hạn
 *   audio.playSFX(SoundType::MOVE);  ← mỗi khi có sự kiện
 *   audio.shutdown();   ← gọi trước khi thoát
 *
 * Lưu ý: AudioManager::handleEvent() nên được gọi trong vòng lặp
 * xử lý sự kiện SDL để tự động phát tiếng click khi người dùng
 * nhấn chuột trên giao diện.
 */

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "GameState.h"

class AudioManager {
private:
    bool initialized; // true sau khi init() thành công.
    bool bgmEnabled;  // true = nhạc nền đang được bật.
    bool sfxEnabled;  // true = hiệu ứng âm thanh đang được bật.

    MIX_Mixer* mixer;     // Mixer chính gắn với audio device.
    MIX_Track* bgmTrack;  // Kênh BGM dùng xuyên suốt ván chơi.
    MIX_Track* sfxTrack;  // Kênh SFX dùng chung cho mọi hiệu ứng.

    MIX_Audio* bgm;      // Dữ liệu file nhạc nền (nhacnen.wav).
    MIX_Audio* moveSfx;  // Âm thanh di chuyển mảnh.

public:
    AudioManager();
    ~AudioManager();

    // ── Vòng đời ─────────────────────────────────────────────────────
    // Khởi tạo SDL audio, SDL_mixer, mở device, load file và tạo track.
    // Trả về false nếu bất kỳ bước nào thất bại.
    bool init();

    // Giải phóng toàn bộ track, audio buffer, mixer và tắt subsystem.
    void shutdown();

    // ── Điều khiển BGM ────────────────────────────────────────────────
    void playBGM();                  // Phát nhạc nền lặp vô hạn.
    void stopBGM();                  // Dừng nhạc nền ngay lập tức.
    void pauseBGM();                 // Tạm dừng (giữ vị trí phát).
    void resumeBGM();                // Tiếp tục từ vị trí đã dừng.
    void setBGMVolume(float volume); // Đặt âm lượng BGM [0.0 .. 1.0].

    // ── Điều khiển SFX ────────────────────────────────────────────────
    // Phát hiệu ứng âm thanh tương ứng sự kiện (không lặp).
    void playSFX(SoundType sound);
    void setSFXVolume(float volume); // Đặt âm lượng SFX [0.0 .. 1.0].

    // ── Cài đặt bật/tắt ───────────────────────────────────────────────
    void toggleBGM();                        // Đảo trạng thái BGM on/off.
    void toggleSFX();                        // Đảo trạng thái SFX on/off.
    bool isBGMEnabled() const;
    bool isSFXEnabled() const;
    void setBGMEnabled(bool enabled);        // Tắt → pause, bật → resume.
    void setSFXEnabled(bool enabled);

    // ── Xử lý sự kiện SDL ────────────────────────────────────────────
    // Gọi trong vòng lặp sự kiện; tự phát tiếng click khi nhấn chuột trái.
    void handleEvent(const SDL_Event& e);

    // ── Debug ─────────────────────────────────────────────────────────
    void printSettings(); // In trạng thái BGM/SFX ra console.
};

#endif