/*
 * Game.h
 * ------
 * Lớp trung tâm điều phối toàn bộ vòng lặp game Tetris.
 *
 * Game sở hữu (owns) tất cả các subsystem:
 *  - SDL_Window / SDL_Renderer : cửa sổ và renderer đồ họa.
 *  - Board                     : lưới các ô đã khóa.
 *  - Tetromino*                : mảnh hiện tại và mảnh tiếp theo.
 *  - Renderer                  : vẽ tất cả thành phần lên màn hình.
 *  - AudioManager              : quản lý BGM và SFX.
 *
 * Vòng lặp game (Game::run()):
 *   Mỗi frame gồm 3 bước theo thứ tự:
 *   1. handleInput()  — xử lý sự kiện SDL (bàn phím, chuột, thoát).
 *   2. update(dt)     — cập nhật vật lý rơi mảnh theo thời gian.
 *   3. render()       — vẽ toàn bộ khung hình lên màn hình.
 *
 * Điều khiển phím (khi đang PLAYING):
 *   ←/→    : di chuyển trái/phải
 *   ↓      : soft drop (+1 điểm/ô)
 *   ↑ / X  : xoay thuận chiều kim đồng hồ
 *   Z      : xoay ngược chiều kim đồng hồ
 *   SPACE  : hard drop (rơi thẳng xuống, +2 điểm/ô)
 *   ESC    : tạm dừng / tiếp tục
 *
 * Chuyển trạng thái (GameState):
 *   MENU → PLAYING → (PAUSED ↔ PLAYING) → GAME_OVER → MENU
 */

#ifndef GAME_H
#define GAME_H

#include "AudioManager.h"
#include "Board.h"
#include "GameState.h"
#include "Renderer.h"
#include "Tetromino.h"
#include <SDL3/SDL.h>

class Game {
private:
    // ── SDL ───────────────────────────────────────────────────────────
    SDL_Window*   window;       // Cửa sổ game.
    SDL_Renderer* sdlRenderer;  // SDL renderer gắn với cửa sổ.

    // ── Trạng thái ────────────────────────────────────────────────────
    GameState state;  // Trạng thái hiện tại của vòng lặp.
    bool      running; // Vòng lặp tiếp tục chạy khi true.

    // ── Các thành phần chính ──────────────────────────────────────────
    Board         board;         // Lưới ô đã khóa.
    Tetromino*    currentPiece;  // Mảnh đang rơi (heap, Game sở hữu).
    Tetromino*    nextPiece;     // Mảnh tiếp theo (heap, Game sở hữu).
    Renderer*     renderer;      // Renderer đồ họa (heap, Game sở hữu).
    AudioManager  audio;         // Hệ thống âm thanh (stack).

    // ── Ba bước xử lý mỗi frame ───────────────────────────────────────
    void handleInput();          // Đọc và xử lý sự kiện SDL.
    void update(float deltaTime);// Cập nhật vật lý (rơi mảnh tự động).
    void render();               // Vẽ toàn bộ khung hình.

    // ── Gameplay logic ────────────────────────────────────────────────
    void spawnPiece();           // Tạo mảnh mới từ nextPiece.
    bool isValidPosition(const Tetromino& piece) const; // Kiểm tra va chạm.
    void lockCurrentPiece();     // Khóa mảnh, xóa hàng, tính điểm.
    void calculateScore(int linesCleared); // Tính điểm và tăng cấp độ.
    void changeState(GameState newState);  // Chuyển trạng thái + xử lý audio.
    void hardDrop();             // Thả mảnh thẳng xuống (SPACE).

    // ── Dữ liệu tiến trình ────────────────────────────────────────────
    int score;      // Điểm tích lũy của ván hiện tại.
    int level;      // Cấp độ hiện tại (tăng mỗi 10 hàng).
    int totalLines; // Tổng số hàng đã xóa.

    // ── Bộ đếm rơi tự động ────────────────────────────────────────────
    float fallTimer;    // Thời gian đã trôi qua kể từ lần rơi cuối (giây).
    float fallInterval; // Khoảng thời gian giữa hai lần rơi (giây).
                        // Bắt đầu = 1.0s, giảm dần theo cấp độ xuống 0.1s.

    // ── Vùng nhấn của các nút giao diện ──────────────────────────────
    // Tọa độ pixel trên cửa sổ; dùng để phát hiện click chuột vào nút.
    SDL_FRect btnPlay     = {370, 305, 230, 55}; // Nút "Chơi" trên menu.
    SDL_FRect btnSettings = {350, 650, 60,  60}; // Nút cài đặt (chưa dùng).
    SDL_FRect btnHelp     = {435, 650, 60,  60}; // Nút trợ giúp (chưa dùng).

    // Trả về true nếu (mouseX, mouseY) nằm trong vùng rect.
    bool isMouseInRect(float mouseX, float mouseY, SDL_FRect rect);

public:
    Game();
    ~Game();

    // ── Vòng đời ─────────────────────────────────────────────────────
    bool init();     // Khởi tạo SDL, tạo cửa sổ, renderer, load assets.
    void run();      // Vòng lặp chính — chạy đến khi người dùng thoát.
    void shutdown(); // Giải phóng toàn bộ tài nguyên.
};

#endif