/*
 * main.cpp
 * --------
 * Điểm vào (entry point) của chương trình.
 *
 * Luồng thực thi:
 *   1. Tạo object Game (constructor đặt các con trỏ về nullptr).
 *   2. game.init()     — khởi tạo SDL, tạo cửa sổ, renderer, load assets;
 *                        trả về false nếu có lỗi → thoát ngay.
 *   3. game.run()      — vào vòng lặp chính (60 FPS) cho đến khi người dùng
 *                        đóng cửa sổ hoặc thoát game.
 *   4. game.shutdown() — giải phóng toàn bộ tài nguyên SDL, hủy cửa sổ.
 *
 * Lưu ý: SDL3 yêu cầu include <SDL3/SDL_main.h> để macro SDL_main
 * hoạt động đúng trên Windows (WinMain → main).
 */

#include "Game.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
    Game game;

    // Khởi tạo toàn bộ hệ thống; nếu thất bại thì dừng ngay.
    if (!game.init()) {
        return 1;
    }

    // Vòng lặp game chính — chạy đến khi người dùng thoát.
    game.run();

    // Dọn dẹp tài nguyên trước khi thoát.
    game.shutdown();

    return 0;
}