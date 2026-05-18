/*
 * Game.cpp — Core game logic: init, run, update, render, gameplay.
 * Input handling nằm trong GameInput.cpp.
 */
#include "Game.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <iostream>
#include <cstring>

// ── Constructor / Destructor ─────────────────────────────────
Game::Game()
    : window(nullptr), sdlRenderer(nullptr), state(GameState::MENU),
      running(false), currentPiece(nullptr), nextPiece(nullptr),
      heldPiece(nullptr), canHold(true), renderer(nullptr),
      score(0), level(1), totalLines(0),
      fallTimer(0.0f), fallInterval(1.0f), animTime(0.0f),
      menuHoveredBtn(-1), selectedLevel(0), levelHovered(-1),
      settingsHovered(-1), editingKeyIndex(-1),
      bgmVolume(1.0f), sfxVolume(1.0f),
      draggingBGM(false), draggingSFX(false) {
    // Init button rects
    float btnW = 280, btnH = 50;
    float btnX = (WINDOW_WIDTH - btnW) / 2.0f;
    btnMenuPlay     = {btnX, 280, btnW, btnH};
    btnMenuSettings = {btnX, 350, btnW, btnH};
    btnMenuTutorial = {btnX, 420, btnW, btnH};

    // Level cards
    float cardW = 240, cardH = 340;
    float totalW = cardW * NUM_LEVELS + 40 * (NUM_LEVELS - 1);
    float startX = (WINDOW_WIDTH - totalW) / 2.0f;
    for (int i = 0; i < NUM_LEVELS; i++)
        btnLevelCards[i] = {startX + i * (cardW + 40), 160, cardW, cardH};
    btnLevelBack = {(WINDOW_WIDTH-200)/2.0f, 570, 200, 45};

    // Settings rects
    bgmSliderRect = {450, 187, 250, 16};
    sfxSliderRect = {450, 232, 250, 16};
    bgmToggleRect = {730, 183, 60, 24};
    sfxToggleRect = {730, 228, 60, 24};
    for (int i = 0; i < 7; i++)
        keyBindRects[i] = {450, (float)(348 + i * 40), 180, 28};
    btnSettingsBack = {(WINDOW_WIDTH-200)/2.0f, 660, 200, 45};
    btnTutorialBack = {(WINDOW_WIDTH-200)/2.0f, 650, 200, 45};

    // Game Over buttons
    float pw = 420, ph = 500;
    float px = (WINDOW_WIDTH - pw) / 2;
    float py = (WINDOW_HEIGHT - ph) / 2;
    float bw = 160, bh = 45;
    float by2 = py + ph - 65;
    int cx = WINDOW_WIDTH / 2;
    btnGORestart = {(float)(cx - bw - 15), by2, bw, bh};
    btnGOMenu    = {(float)(cx + 15), by2, bw, bh};
}

Game::~Game() {
    if (running) shutdown();
}

// ── init ─────────────────────────────────────────────────────
bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << std::endl;
        SDL_Quit(); return false;
    }
    sdlRenderer = SDL_CreateRenderer(window, NULL);
    if (!sdlRenderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window); SDL_Quit(); return false;
    }
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    renderer = new Renderer(sdlRenderer);
    if (!audio.init()) {
        std::cerr << "Audio init failed: " << SDL_GetError() << std::endl;
    } else {
        audio.playBGM();
    }
    running = true;
    state = GameState::MENU;
    return true;
}

// ── run ──────────────────────────────────────────────────────
void Game::run() {
    Uint64 lastTime = SDL_GetTicks();
    const float FRAME_DURATION = 1000.0f / 60.0f;
    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        animTime += deltaTime;
        handleInput();
        update(deltaTime);
        render();
        Uint64 frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < (Uint64)FRAME_DURATION)
            SDL_Delay((Uint32)(FRAME_DURATION - frameTime));
    }
}

// ── shutdown ─────────────────────────────────────────────────
void Game::shutdown() {
    delete currentPiece; currentPiece = nullptr;
    delete nextPiece;    nextPiece    = nullptr;
    delete heldPiece;    heldPiece    = nullptr;
    delete renderer;     renderer     = nullptr;
    audio.shutdown();
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// ── update ───────────────────────────────────────────────────
void Game::update(float deltaTime) {
    if (state != GameState::PLAYING) return;
    fallTimer += deltaTime;
    if (fallTimer >= fallInterval) {
        fallTimer = 0.0f;
        if (currentPiece) {
            currentPiece->moveDown();
            if (!isValidPosition(*currentPiece)) {
                currentPiece->moveUp();
                lockCurrentPiece();
            }
        }
    }
}

// ── render ───────────────────────────────────────────────────
void Game::render() {
    renderer->clear();

    switch (state) {
    case GameState::MENU:
        renderer->drawMenuScreen(menuHoveredBtn, animTime);
        break;

    case GameState::LEVEL_SELECT:
        renderer->drawLevelSelectScreen(selectedLevel, levelHovered);
        break;

    case GameState::SETTINGS: {
        const char* kn[7];
        kn[0] = getKeyName(keys.moveLeft);
        kn[1] = getKeyName(keys.moveRight);
        kn[2] = getKeyName(keys.softDrop);
        kn[3] = getKeyName(keys.hardDrop);
        kn[4] = getKeyName(keys.rotateCW);
        kn[5] = getKeyName(keys.rotateCCW);
        kn[6] = getKeyName(keys.hold);
        renderer->drawSettingsScreen(bgmVolume, sfxVolume,
            audio.isBGMEnabled(), audio.isSFXEnabled(),
            settingsHovered, editingKeyIndex, kn);
        break;
    }

    case GameState::TUTORIAL: {
        const char* kn[7];
        kn[0] = getKeyName(keys.moveLeft);
        kn[1] = getKeyName(keys.moveRight);
        kn[2] = getKeyName(keys.softDrop);
        kn[3] = getKeyName(keys.hardDrop);
        kn[4] = getKeyName(keys.rotateCW);
        kn[5] = getKeyName(keys.rotateCCW);
        kn[6] = getKeyName(keys.hold);
        renderer->drawTutorialScreen(kn);
        break;
    }

    case GameState::PLAYING:
    case GameState::PAUSED:
    case GameState::GAME_OVER:
        renderer->drawGameBackground(level);
        renderer->drawBoard(board);
        if (state == GameState::PLAYING && currentPiece) {
            renderer->drawGhostPiece(*currentPiece, currentPiece->getGhostY(board));
            renderer->drawTetromino(*currentPiece);
        }
        if (nextPiece) renderer->drawNextPiece(*nextPiece);
        if (heldPiece) renderer->drawHeldPiece(*heldPiece);
        renderer->drawUI(score, level, totalLines);
        if (state == GameState::PAUSED)
            renderer->drawPauseOverlay();
        if (state == GameState::GAME_OVER)
            renderer->drawGameOverScreen(score, level, totalLines);
        break;
    }

    renderer->present();
}

// ── startGame ────────────────────────────────────────────────
void Game::startGame(int levelIndex) {
    board.reset();
    score = 0;
    totalLines = 0;
    level = LEVEL_CONFIGS[levelIndex].startLevel;
    fallInterval = LEVEL_CONFIGS[levelIndex].startFallInterval;
    delete currentPiece; currentPiece = nullptr;
    delete nextPiece;    nextPiece = nullptr;
    delete heldPiece;    heldPiece = nullptr;
    canHold = true;
    spawnPiece();
    changeState(GameState::PLAYING);
}

// ── spawnPiece ───────────────────────────────────────────────
void Game::spawnPiece() {
    if (nextPiece == nullptr)
        nextPiece = new Tetromino(Tetromino::createRandom());
    canHold = true;
    delete currentPiece;
    currentPiece = nextPiece;
    nextPiece = new Tetromino(Tetromino::createRandom());
    if (!isValidPosition(*currentPiece))
        changeState(GameState::GAME_OVER);
}

// ── holdPiece ────────────────────────────────────────────────
void Game::holdPiece() {
    if (!canHold || !currentPiece) return;
    if (heldPiece == nullptr) {
        heldPiece = new Tetromino(currentPiece->getType());
        delete currentPiece; currentPiece = nullptr;
        spawnPiece();
    } else {
        TetrominoType ct = currentPiece->getType();
        TetrominoType ht = heldPiece->getType();
        delete currentPiece; currentPiece = new Tetromino(ht);
        delete heldPiece;    heldPiece = new Tetromino(ct);
        if (!isValidPosition(*currentPiece))
            changeState(GameState::GAME_OVER);
    }
    canHold = false;
    audio.playSFX(SoundType::MOVE);
}

// ── hardDrop ─────────────────────────────────────────────────
void Game::hardDrop() {
    if (!currentPiece) return;
    int dropped = 0;
    while (isValidPosition(*currentPiece)) {
        currentPiece->moveDown(); dropped++;
    }
    currentPiece->moveUp(); dropped--;
    score += dropped * 2;
    lockCurrentPiece();
}

// ── isValidPosition ──────────────────────────────────────────
bool Game::isValidPosition(const Tetromino& piece) const {
    for (int r = 0; r < TETROMINO_SIZE; ++r)
        for (int c = 0; c < TETROMINO_SIZE; ++c) {
            if (!piece.isCellFilled(c, r)) continue;
            int bx = piece.x + c, by = piece.y + r;
            if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return false;
            if (by >= 0 && !board.isCellEmpty(bx, by)) return false;
        }
    return true;
}

// ── tryRotateCW / tryRotateCCW  (SRS Wall Kick) ──────────────
// Xoay khối chống kẹt tường (Wall Kick) dựa theo Super Rotation System (SRS).
// Thay vì cấm xoay khi nằm sát tường, thuật toán sẽ thử tịnh tiến (dịch chuyển)
// khối gạch sang trái, phải, hoặc đẩy lên trên thông qua mảng tọa độ "kick".
//
// Bảng Wall Kick theo Tetris Guideline (SRS):
//   - Mảnh J, L, S, T, Z dùng bảng JLSTZ.
//   - Mảnh I dùng bảng riêng vì kích thước 4 ô.
//   - Mảnh O không cần (xoay không thay đổi hình dạng).
//
// Mỗi hàng là một cặp (dx, dy) thử theo thứ tự; nếu vị trí hợp lệ
// thì áp dụng offset đó và trả về true. Nếu tất cả đều thất bại,
// hoàn tác xoay và trả về false.

bool Game::tryRotateCW() {
    if (!currentPiece) return false;

    // O piece: hình vuông xoay ra y hệt — không xoay, tránh ma trận dịch chỗ.
    if (currentPiece->getType() == TetrominoType::O) return true;

    const int fromRot = currentPiece->getRotation();
    currentPiece->rotateCW();

    // Bảng kick cho JLSTZ: [from_rotation][test_index] = {dx, dy}
    static const int kickJLSTZ[4][5][2] = {
        // 0→1
        {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}},
        // 1→2
        {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}},
        // 2→3
        {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}},
        // 3→0
        {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}},
    };

    // Bảng kick cho mảnh I: [from_rotation][test_index] = {dx, dy}
    static const int kickI[4][5][2] = {
        // 0→1
        {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}},
        // 1→2
        {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}},
        // 2→3
        {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}},
        // 3→0
        {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}},
    };

    const bool isIPiece = (currentPiece->getType() == TetrominoType::I);
    const auto& table = isIPiece ? kickI : kickJLSTZ;

    for (int t = 0; t < 5; ++t) {
        currentPiece->x += table[fromRot][t][0];
        currentPiece->y -= table[fromRot][t][1]; // dy trong SRS: dương = lên
        if (isValidPosition(*currentPiece)) return true;
        // Hoàn tác offset, thử test tiếp theo
        currentPiece->x -= table[fromRot][t][0];
        currentPiece->y += table[fromRot][t][1];
    }

    // Tất cả test đều thất bại — hoàn tác xoay
    currentPiece->rotateCCW();
    return false;
}

bool Game::tryRotateCCW() {
    if (!currentPiece) return false;

    // O piece: hình vuông xoay ra y hệt — không xoay.
    if (currentPiece->getType() == TetrominoType::O) return true;

    const int fromRot = currentPiece->getRotation();
    currentPiece->rotateCCW();

    // Bảng kick CCW cho JLSTZ (nghịch đảo của CW):
    static const int kickJLSTZ_CCW[4][5][2] = {
        // 0→3
        {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}},
        // 1→0
        {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}},
        // 2→1
        {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}},
        // 3→2
        {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}},
    };

    static const int kickI_CCW[4][5][2] = {
        // 0→3
        {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}},
        // 1→0
        {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}},
        // 2→1
        {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}},
        // 3→2
        {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}},
    };

    const bool isIPiece = (currentPiece->getType() == TetrominoType::I);
    const auto& table = isIPiece ? kickI_CCW : kickJLSTZ_CCW;

    for (int t = 0; t < 5; ++t) {
        currentPiece->x += table[fromRot][t][0];
        currentPiece->y -= table[fromRot][t][1];
        if (isValidPosition(*currentPiece)) return true;
        currentPiece->x -= table[fromRot][t][0];
        currentPiece->y += table[fromRot][t][1];
    }

    currentPiece->rotateCW();
    return false;
}

// ── lockCurrentPiece ─────────────────────────────────────────
void Game::lockCurrentPiece() {
    if (!currentPiece) return;
    board.lockPiece(*currentPiece);
    audio.playSFX(SoundType::LOCK);
    int lc = board.clearLines();
    if (lc > 0) {
        switch (lc) {
            case 1: audio.playSFX(SoundType::CLEAR_SINGLE); break;
            case 2: audio.playSFX(SoundType::CLEAR_DOUBLE); break;
            case 3: audio.playSFX(SoundType::CLEAR_TRIPLE); break;
            case 4: audio.playSFX(SoundType::CLEAR_TETRIS); break;
        }
        calculateScore(lc);
    }
    spawnPiece();
}

// ── calculateScore ───────────────────────────────────────────
void Game::calculateScore(int linesCleared) {
    totalLines += linesCleared;
    int points = 0;
    switch (linesCleared) {
        case 1: points = 100; break;
        case 2: points = 300; break;
        case 3: points = 500; break;
        case 4: points = 800; break;
    }
    score += points * level;
    level = (totalLines / 10) + LEVEL_CONFIGS[selectedLevel].startLevel;
    fallInterval = std::max(0.1f, LEVEL_CONFIGS[selectedLevel].startFallInterval - (level - LEVEL_CONFIGS[selectedLevel].startLevel) * 0.1f);
}

// ── changeState ──────────────────────────────────────────────
void Game::changeState(GameState newState) {
    if (state == newState) return;
    if (newState == GameState::GAME_OVER) {
        audio.pauseBGM();
        audio.playSFX(SoundType::GAME_OVER);
    }
    if (state == GameState::GAME_OVER && newState != GameState::GAME_OVER)
        audio.resumeBGM();
    state = newState;
}

// ── isMouseInRect ────────────────────────────────────────────
bool Game::isMouseInRect(float mx, float my, SDL_FRect r) {
    return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h;
}

// ── getKeyName ───────────────────────────────────────────────
const char* Game::getKeyName(SDL_Keycode key) {
    return SDL_GetKeyName(key);
}