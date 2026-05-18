/*
 * Game.h
 * ------
 * Lớp trung tâm điều phối toàn bộ vòng lặp game Tetris.
 * Hỗ trợ: Menu, Level Select, Settings, Tutorial, Playing, Paused, Game Over.
 */

#ifndef GAME_H
#define GAME_H

#include "AudioManager.h"
#include "Board.h"
#include "GameState.h"
#include "Renderer.h"
#include "Tetromino.h"
#include <SDL3/SDL.h>

// Key bindings — 6 phím có thể tùy chỉnh
struct KeyBindings {
    SDL_Keycode moveLeft  = SDLK_LEFT;
    SDL_Keycode moveRight = SDLK_RIGHT;
    SDL_Keycode softDrop  = SDLK_DOWN;
    SDL_Keycode hardDrop  = SDLK_SPACE;
    SDL_Keycode rotateCW  = SDLK_UP;
    SDL_Keycode rotateCCW = SDLK_Z;
    SDL_Keycode hold      = SDLK_C;
};

class Game {
private:
    // ── SDL ───────────────────────────────────────────────────────────
    SDL_Window*   window;
    SDL_Renderer* sdlRenderer;

    // ── Trạng thái ────────────────────────────────────────────────────
    GameState state;
    bool      running;

    // ── Các thành phần chính ──────────────────────────────────────────
    Board         board;
    Tetromino*    currentPiece;
    Tetromino*    nextPiece;
    Tetromino*    heldPiece;
    bool          canHold;
    Renderer*     renderer;
    AudioManager  audio;

    // ── Frame processing ─────────────────────────────────────────────
    void handleInput();
    void update(float deltaTime);
    void render();

    // ── Gameplay logic ───────────────────────────────────────────────
    void spawnPiece();
    bool isValidPosition(const Tetromino& piece) const;
    void lockCurrentPiece();
    void calculateScore(int linesCleared);
    void changeState(GameState newState);
    void hardDrop();
    void holdPiece();
    void startGame(int levelIndex);
    bool tryRotateCW();   // Xoay CW với wall kick (SRS)
    bool tryRotateCCW();  // Xoay CCW với wall kick (SRS)

    // ── Dữ liệu tiến trình ──────────────────────────────────────────
    int score;
    int level;
    int totalLines;
    float fallTimer;
    float fallInterval;
    float animTime;  // Thời gian animation cho menu background

    // ── UI State ─────────────────────────────────────────────────────
    int menuHoveredBtn;      // 0=Play, 1=Settings, 2=Tutorial, -1=none
    int selectedLevel;       // 0..2 (index trong LEVEL_CONFIGS)
    int levelHovered;        // 0..2 + 3=back, -1=none
    int settingsHovered;     // 0=bgmSlider,1=bgmToggle,2=sfxSlider,3=sfxToggle, 4-10=keys, 11=back
    int editingKeyIndex;     // -1=none, 0-6=đang chờ nhấn phím mới
    float bgmVolume;
    float sfxVolume;

    // ── Key Bindings ─────────────────────────────────────────────────
    KeyBindings keys;
    const char* getKeyName(SDL_Keycode key);

    // ── Button rects (tính toán tại render) ──────────────────────────
    SDL_FRect btnMenuPlay, btnMenuSettings, btnMenuTutorial;
    SDL_FRect btnLevelCards[NUM_LEVELS];
    SDL_FRect btnLevelBack;
    SDL_FRect btnSettingsBack;
    SDL_FRect btnTutorialBack;
    SDL_FRect btnGORestart, btnGOMenu;

    bool isMouseInRect(float mouseX, float mouseY, SDL_FRect rect);

    // ── Settings slider rects ────────────────────────────────────────
    SDL_FRect bgmSliderRect, sfxSliderRect;
    SDL_FRect bgmToggleRect, sfxToggleRect;
    SDL_FRect keyBindRects[7];
    bool draggingBGM, draggingSFX;

public:
    Game();
    ~Game();
    bool init();
    void run();
    void shutdown();
};

#endif