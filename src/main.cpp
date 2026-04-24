#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Game.h"

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init()) {
        return 1;
    }

    game.run();
    game.shutdown();
    /*Game game — tạo object, constructor chạy, khởi tạo con trỏ về nullptr
    game.init() — khởi động SDL, tạo window, renderer, load assets — trả về false nếu thất bại
    game.run() — vào vòng lặp chính, chạy cho đến khi người dùng thoát
    game.shutdown() — dọn dẹp mọi thứ sau khi thoát vòng lặp*/
    return 0;
}