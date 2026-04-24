#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL3/SDL.h>
#include "GameState.h"

class AudioManager {
    private:
        bool initialized;
        /*initialized — lưu trạng thái khởi tạo, nếu SDL audio init thất bại thì các hàm khác biết mà bỏ qua, không crash
         TV4 sẽ thêm các con trỏ sound buffer vào private sau*/
        bool bgmEnabled;   // ← thêm dòng này
        bool sfxEnabled;   // ← thêm dòng này
    public:
        AudioManager();
        ~AudioManager();

        // Hàm khởi tạo & dọn dẹp
        // Khởi tạo SDL audio, load toàn bộ file âm thanh
        bool init();

        // Giải phóng tất cả âm thanh
        void shutdown();

        // Hàm nhạc nền BGM
        void playBGM();
        void stopBGM();
        void pauseBGM();
        void resumeBGM();

        // Điều chỉnh âm lượng nhạc nền (0-128)
        void setBGMVolume(int volume);

        // Hàm hiệu ứng âm thanh SFX
        // Phát 1 hiệu ứng âm thanh
        void playSFX(SoundType sound);

        // Điều chỉnh âm lượng SFX (0-128)
        void setSFXVolume(int volume);

        // Hàm toggle (dùng cho Settings)
        void toggleBGM();
        void toggleSFX();

        bool isBGMEnabled() const;
        bool isSFXEnabled() const;

};

#endif