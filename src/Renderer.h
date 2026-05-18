/*
 * Renderer.h
 * ----------
 * Lớp Renderer — vẽ toàn bộ giao diện game bằng SDL3 primitives + SDL3_ttf.
 * Không dùng ảnh nền — mọi thứ được vẽ bằng code với phong cách pixel/neon.
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "GameState.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

class Board;
class Tetromino;

class Renderer {
private:
    SDL_Renderer* sdlRenderer;

    // ── Fonts (pixel style) ──────────────────────────────────────────
    TTF_Font* fontLarge;   // 32px — tiêu đề
    TTF_Font* fontMedium;  // 20px — nút, label
    TTF_Font* fontSmall;   // 14px — mô tả, giá trị

    // ── Highscore ────────────────────────────────────────────────────
    std::vector<int> highscores;
    bool highscoresLoaded;
    void loadHighscores();

    // ── Tiện ích vẽ ──────────────────────────────────────────────────
    void renderText(TTF_Font* f, const char* text, int x, int y, SDL_Color color);
    void renderTextCentered(TTF_Font* f, const char* text, int cx, int cy, SDL_Color color);
    SDL_Color getTetrominoColor(TetrominoType type);

    // Tính chiều rộng text (dùng cho layout)
    int getTextWidth(TTF_Font* f, const char* text);

public:
    Renderer(SDL_Renderer* renderer);
    ~Renderer();

    // ── Frame lifecycle ──────────────────────────────────────────────
    void clear();
    void present();

    // ── UI Primitives ────────────────────────────────────────────────
    void drawBlock3D(float x, float y, float size, SDL_Color color);
    void drawPanel(float x, float y, float w, float h,
                   SDL_Color bg, SDL_Color border);
    void drawButton(const char* text, SDL_FRect rect, bool hovered,
                    SDL_Color accent = {233, 69, 96, 255});

    // ── Screens ──────────────────────────────────────────────────────
    void drawMenuScreen(int hoveredBtn, float animTime);
    void drawLevelSelectScreen(int selectedLevel, int hoveredLevel);
    void drawSettingsScreen(float bgmVol, float sfxVol, bool bgmOn, bool sfxOn,
                            int hoveredItem, int editingKey,
                            const char* keyNames[7]);
    void drawTutorialScreen(const char* keyNames[7]);
    void drawPauseOverlay();
    void drawGameOverScreen(int score, int level, int lines);

    // ── Game components ──────────────────────────────────────────────
    void drawGameBackground(int currentLevel);
    void drawBoard(const Board& board);
    void drawTetromino(const Tetromino& tetromino);
    void drawGhostPiece(const Tetromino& tetromino, int ghostY);
    void drawNextPiece(const Tetromino& nextPiece);
    void drawHeldPiece(const Tetromino& heldPiece);
    void drawUI(int score, int level, int lines);
    void drawLineClearEffect(const Board& board, int clearedRows[], int count);
};

#endif