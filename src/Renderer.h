/*
 * Renderer.h
 * ----------
 * Lớp Renderer — chịu trách nhiệm vẽ toàn bộ giao diện game lên màn hình
 * bằng SDL3 + SDL3_image + SDL3_ttf.
 *
 * Tài nguyên được quản lý:
 *  - mainMenuTexture  : nền màn hình chính (main_menu.png).
 *  - gameScreenTexture: nền bảng chơi (gamescreen.png).
 *  - highScoreTexture : popup game over / bảng điểm (highscore.png).
 *  - blockTexture     : sprite block đơn (block_green.png).
 *                       Màu được thay đổi runtime bằng SDL_SetTextureColorMod.
 *  - font             : font chữ TTF dùng để hiển thị số điểm, level.
 *
 * Hệ thống highscore:
 *  - Lưu tối đa 5 điểm cao nhất vào file "highscores.txt".
 *  - Load và lưu tự động mỗi khi vào màn hình GAME_OVER.
 *
 * Quy ước tọa độ:
 *  - Góc trên-trái màn hình = (0, 0).
 *  - Bảng chơi bắt đầu tại (BOARD_OFFSET_X, BOARD_OFFSET_Y).
 *  - Panel "Next" bắt đầu tại (730, 183).
 *  - Panel thông tin (điểm/level/lines) tại cột x = 177.
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "GameState.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

// Khai báo trước để tránh include vòng.
class Board;
class Tetromino;

class Renderer {
private:
    SDL_Renderer* sdlRenderer;       // SDL Renderer dùng để vẽ.
    SDL_Texture*  mainMenuTexture;   // Nền màn hình chính.
    SDL_Texture*  gameScreenTexture; // Nền bảng chơi.
    SDL_Texture*  highScoreTexture;  // Popup game over.
    SDL_Texture*  blockTexture;      // Sprite ô vuông (tô màu lúc vẽ).
    TTF_Font*     font;              // Font chữ cho số điểm, level, v.v.

    std::vector<int> highscores;  // Danh sách ≤5 điểm cao nhất.
    bool highscoresLoaded;        // Cờ tránh đọc file nhiều lần trong 1 ván.

    // Đọc highscores từ file, sắp xếp giảm dần, giữ tối đa 5 điểm.
    void loadHighscores();

    // ── Hàm tiện ích nội bộ ──────────────────────────────────────────
    // Vẽ text tại (x, y) với màu cho trước.
    void renderText(const char* text, int x, int y, SDL_Color color);

    // Vẽ text căn giữa theo tọa độ tâm (cx, cy).
    void renderTextCentered(const char* text, int cx, int cy, SDL_Color color);

    // Trả về màu SDL tương ứng loại mảnh Tetromino.
    SDL_Color getTetrominoColor(TetrominoType type);

public:
    // Constructor: nhận SDL_Renderer từ Game, load tất cả tài nguyên.
    Renderer(SDL_Renderer* renderer);

    // Destructor: hủy texture, đóng font, gọi TTF_Quit.
    ~Renderer();

    // ── Frame lifecycle ───────────────────────────────────────────────
    void clear();    // Xóa màn hình (nền đen), bắt đầu frame mới.
    void present();  // Hiển thị frame đã vẽ lên màn hình.

    // ── Vẽ thành phần game ────────────────────────────────────────────
    // Vẽ nền bảng chơi và tất cả các ô đã khóa.
    void drawBoard(const Board& board);

    // Vẽ mảnh Tetromino đang rơi với màu tương ứng loại mảnh.
    void drawTetromino(const Tetromino& tetromino);

    // Vẽ bóng mờ (ghost piece) tại vị trí hạ cánh dự kiến (ghostY).
    void drawGhostPiece(const Tetromino& tetromino, int ghostY);

    // Vẽ mảnh tiếp theo ở panel bên phải bảng chơi.
    void drawNextPiece(const Tetromino& nextPiece);

    // Vẽ thông tin UI: điểm, cấp độ, số hàng đã xóa.
    void drawUI(int score, int level, int lines);

    // Vẽ màn hình theo trạng thái GameState:
    //   MENU      → vẽ ảnh main menu.
    //   PAUSED    → hiển thị chữ "PAUSED".
    //   GAME_OVER → hiển thị popup với bảng điểm cao nhất.
    void drawScreen(GameState state, int currentScore = 0,
                    int currentLevel = 0, int currentLines = 0);

    // ── Hiệu ứng ─────────────────────────────────────────────────────
    // Flash (nháy sáng) các hàng được xóa trước khi xóa thật sự.
    void drawLineClearEffect(const Board& board, int clearedRows[], int count);
};

#endif