#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include "GameState.h"
#include "Board.h"
#include "Tetromino.h"
#include "Renderer.h"
#include "AudioManager.h"

class Game {
    private:
        // SDL
        SDL_Window*   window;
        SDL_Renderer* sdlRenderer;

        // Trạng thái game
        GameState state;
        bool      running;

        // Các thành phần chính
        Board        board;
        Tetromino*   currentPiece; 
        Tetromino*   nextPiece;
        Renderer*    renderer;
        AudioManager audio; 
        // currentPiece và nextPiece dùng con trỏ vì chúng thay đổi liên tục trong game
        // board và audio dùng trực tiếp (không phải con trỏ) vì chỉ có 1 và tồn tại suốt game
        // renderer dùng con trỏ vì cần sdlRenderer khởi tạo xong mới tạo được

        // Hàm game loop 
        // 3 bước của mỗi frame
        void handleInput();
        void update(float deltaTime);
        void render();

        // Hàm game logic
        // Spawn mảnh mới từ nextPiece, tạo nextPiece mới
        void spawnPiece();

        // Kiểm tra va chạm trước khi di chuyển/xoay
        bool isValidPosition(const Tetromino& piece) const;

        // Khóa mảnh vào board, xóa dòng, tính điểm, spawn tiếp
        void lockCurrentPiece();

        // Tính điểm dựa trên số dòng xóa được
        void calculateScore(int linesCleared);

        // Chuyển đổi state
        void changeState(GameState newState);

        // Dữ liệu game
        // Điểm số
        int score;
        int level;
        int totalLines;

        // Timer để điều khiển tốc độ rơi
        float fallTimer;      // đếm thời gian từ lần rơi cuối
        float fallInterval;   // khoảng cách giữa mỗi lần rơi (giây)
    
    public:
        Game();
        ~Game();

        // Hàm vòng đời (lifecycle)
        // Khởi tạo SDL, tạo window, tạo renderer, load assets
        bool init();

        // Vòng lặp chính — chạy cho đến khi thoát
        void run();

        // Dọn dẹp toàn bộ
        void shutdown();
};

#endif