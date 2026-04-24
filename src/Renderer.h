#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include "GameState.h"

// Forward declaration — tránh circular include
class Board;
class Tetromino;

class Renderer {
    private:
        SDL_Renderer* sdlRenderer;
    public:
        Renderer(SDL_Renderer* renderer);
        ~Renderer();

        // Hàm vẽ từng thành phần
        // Xóa màn hình, chuẩn bị frame mới
        void clear();

        // Hiển thị frame lên màn hình
        void present();

        // Vẽ board (các ô đã bị lock)
        void drawBoard(const Board& board);

        // Vẽ mảnh đang rơi
        void drawTetromino(const Tetromino& tetromino);

        // Vẽ ghost piece (bóng mờ)
        void drawGhostPiece(const Tetromino& tetromino, int ghostY);

        // Vẽ mảnh tiếp theo ở panel bên phải
        void drawNextPiece(const Tetromino& nextPiece);

        // Hàm vẽ UI (điểm số, level)
        // Vẽ toàn bộ UI: score, level, lines
        void drawUI(int score, int level, int lines);

        // Vẽ màn hình theo state hiện tại
        void drawScreen(GameState state);
        /*Nhận vào GameState — tự biết vẽ màn hình nào
        MENU → vẽ main menu
        PAUSED → vẽ overlay pause
        GAME_OVER → vẽ màn hình game over*/

        // Hàm animation
        // Flash các dòng trước khi xóa
        void drawLineClearEffect(const Board& board, int clearedRows[], int count);
};

#endif