#include "Game.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <iostream>

Game::Game()
    : window(nullptr), sdlRenderer(nullptr), state(GameState::MENU),
      running(false), currentPiece(nullptr), nextPiece(nullptr),
      renderer(nullptr), score(0), level(1), totalLines(0),
      fallTimer(0.0f), fallInterval(1.0f) {}

bool Game::init() {
    // SDL_Init trả về 0 khi thành công, <0 khi lỗi.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    sdlRenderer = SDL_CreateRenderer(window, NULL);
    if (!sdlRenderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Bat alpha blending cho renderer de ve bong mo dung.
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

void Game::run() {
    Uint64 lastTime = SDL_GetTicks();

    // Gioi han 60 FPS de giam tai CPU va on dinh deltaTime.
    const float TARGET_FPS     = 60.0f;
    const float FRAME_DURATION = 1000.0f / TARGET_FPS; // ~16.67ms mỗi frame

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime    = (currentTime - lastTime) / 1000.0f;
        lastTime           = currentTime;

        // Giới hạn deltaTime tối đa — tránh "spiral of death" khi lag đột ngột
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        handleInput();
        update(deltaTime);
        render();

        // ✅ Tính thời gian còn lại của frame rồi delay
        Uint64 frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < (Uint64)FRAME_DURATION) {
            SDL_Delay((Uint32)(FRAME_DURATION - frameTime));
        }
    }
}

void Game::shutdown() {
    delete currentPiece;
    currentPiece = nullptr;
    delete nextPiece;
    nextPiece = nullptr;
    delete renderer;
    renderer = nullptr;

    audio.shutdown();

    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

Game::~Game() {
    if (running) {
        shutdown();
    }
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        audio.handleEvent(event);

        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
            const int mouseX = event.button.x;
            const int mouseY = event.button.y;

            if (state == GameState::MENU && isMouseInRect(mouseX, mouseY, btnPlay)) {
                board.reset();
                score = 0; level = 1; totalLines = 0; fallInterval = 1.0f;
                delete currentPiece; currentPiece = nullptr;
                delete nextPiece;    nextPiece    = nullptr;
                spawnPiece();
                changeState(GameState::PLAYING);
            }

            if (state == GameState::GAME_OVER) {
                const float popupX = (float)(WINDOW_WIDTH / 2 - 210);
                const float popupY = (float)(WINDOW_HEIGHT / 2 - 200);

                // Nut xoay (choi lai) o ben trai popup.
                SDL_FRect btnRestart = {popupX + 174.0f, popupY + 308.0f, 72.0f, 72.0f};
                // Nut home (ve menu) o ben phai popup.
                SDL_FRect btnHome    = {popupX + 92.0f, popupY + 308.0f, 72.0f, 72.0f};

                if (isMouseInRect(mouseX, mouseY, btnRestart)) {
                    board.reset();
                    score = 0; level = 1; totalLines = 0; fallInterval = 1.0f;
                    delete currentPiece; currentPiece = nullptr;
                    delete nextPiece;    nextPiece    = nullptr;
                    spawnPiece();
                    changeState(GameState::PLAYING);
                } else if (isMouseInRect(mouseX, mouseY, btnHome)) {
                    changeState(GameState::MENU);
                }
            }
        }

        else if (event.type == SDL_EVENT_KEY_DOWN) {

            // Phim ESC: chuyen qua lai trang thai tam dung.
            if (event.key.key == SDLK_ESCAPE) {
                if (state == GameState::PLAYING)
                    changeState(GameState::PAUSED);
                else if (state == GameState::PAUSED)
                    changeState(GameState::PLAYING);
            }

            // O menu: nhan ENTER de bat dau.
            if (state == GameState::MENU && event.key.key == SDLK_RETURN) {
                board.reset();
                score = 0; level = 1; totalLines = 0; fallInterval = 1.0f;
                delete currentPiece; currentPiece = nullptr;
                delete nextPiece;    nextPiece    = nullptr;
                spawnPiece();
                changeState(GameState::PLAYING);
            }

            // Khi thua: nhan ENTER de ve menu.
            else if (state == GameState::GAME_OVER && event.key.key == SDLK_RETURN) {
                changeState(GameState::MENU);
            }

            // Các phím chỉ hoạt động khi đang PLAYING
            if (state != GameState::PLAYING || currentPiece == nullptr)
                continue;

            switch (event.key.key) {
            case SDLK_LEFT:
                currentPiece->moveLeft();
                if (!isValidPosition(*currentPiece)) currentPiece->moveRight();
                else audio.playSFX(SoundType::MOVE);
                break;

            case SDLK_RIGHT:
                currentPiece->moveRight();
                if (!isValidPosition(*currentPiece)) currentPiece->moveLeft();
                else audio.playSFX(SoundType::MOVE);
                break;

            case SDLK_DOWN:
                // Soft drop: di chuyển xuống 1 ô, +1 điểm
                currentPiece->moveDown();
                if (!isValidPosition(*currentPiece)) {
                    currentPiece->moveUp(); // ✅ FIX 3: dùng moveUp() thay vì y -= 1
                    lockCurrentPiece();
                } else {
                    score += 1;
                    audio.playSFX(SoundType::MOVE);
                }
                break;

            case SDLK_UP:
            case SDLK_X:
                currentPiece->rotateCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCCW();
                else audio.playSFX(SoundType::ROTATE);
                break;

            case SDLK_Z:
                currentPiece->rotateCCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCW();
                else audio.playSFX(SoundType::ROTATE);
                break;

            case SDLK_SPACE:
                hardDrop(); // Hard drop duoc tach rieng thanh ham.
                break;
            }
        }
    }
}

// Hard drop duoc tach rieng de de doc va tai su dung.
void Game::hardDrop() {
    if (!currentPiece) return;

    // Dem so o roi xuong de tinh diem (2 diem/o).
    int cellsDropped = 0;
    while (isValidPosition(*currentPiece)) {
        currentPiece->moveDown();
        cellsDropped++;
    }
    // Hoan tac buoc cuoi vi buoc do da lam manh vao vi tri khong hop le.
    currentPiece->moveUp(); // Dung moveUp() de giu logic di chuyen dong nhat.
    cellsDropped--;

    score += cellsDropped * 2;
    lockCurrentPiece();
}

void Game::update(float deltaTime) {
    if (state != GameState::PLAYING) return;

    fallTimer += deltaTime;
    if (fallTimer >= fallInterval) {
        fallTimer = 0.0f;

        if (currentPiece) {
            currentPiece->moveDown();
            if (!isValidPosition(*currentPiece)) {
                currentPiece->moveUp(); // Dung moveUp() de hoan tac buoc roi.
                lockCurrentPiece();
            }
        }
    }
}

void Game::render() {
    renderer->clear();

    // Ve game khi dang choi, tam dung, hoac da thua.
    if (state == GameState::PLAYING ||
        state == GameState::PAUSED   ||
        state == GameState::GAME_OVER) {

        renderer->drawBoard(board);

        // Chi ve manh roi khi dang PLAYING.
        if (state == GameState::PLAYING && currentPiece) {
            renderer->drawGhostPiece(*currentPiece, currentPiece->getGhostY(board));
            renderer->drawTetromino(*currentPiece);
        }

        if (nextPiece) renderer->drawNextPiece(*nextPiece);
        renderer->drawUI(score, level, totalLines);
    }

    // Ve lop phu theo tung trang thai.
    if (state != GameState::PLAYING) {
        renderer->drawScreen(state, score, level, totalLines);
    }

    renderer->present();
}

void Game::spawnPiece() {
    // Lan dau chay neu chua co nextPiece thi tao moi.
    if (nextPiece == nullptr) {
        nextPiece = new Tetromino(Tetromino::createRandom());
    }

    delete currentPiece;
    currentPiece = nextPiece;
    nextPiece    = new Tetromino(Tetromino::createRandom());

    // Neu vi tri xuat hien da bi chan thi ket thuc van.
    if (!isValidPosition(*currentPiece)) {
        changeState(GameState::GAME_OVER);
    }
}

bool Game::isValidPosition(const Tetromino& piece) const {
    for (int r = 0; r < TETROMINO_SIZE; ++r) {
        for (int c = 0; c < TETROMINO_SIZE; ++c) {
            if (!piece.isCellFilled(c, r)) continue;

            int boardX = piece.x + c;
            int boardY = piece.y + r;

            // Kiem tra bien trai, phai va day.
            if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT)
                return false;

            // Kiem tra va cham voi o da co manh (boardY >= 0 cho phep xuat hien phia tren).
            if (boardY >= 0 && !board.isCellEmpty(boardX, boardY))
                return false;
        }
    }
    return true;
}

void Game::lockCurrentPiece() {
    if (!currentPiece) return;

    board.lockPiece(*currentPiece);
    audio.playSFX(SoundType::LOCK);
    int linesCleared = board.clearLines();
    if (linesCleared > 0) {
        switch (linesCleared) {
            case 1: audio.playSFX(SoundType::CLEAR_SINGLE); break;
            case 2: audio.playSFX(SoundType::CLEAR_DOUBLE); break;
            case 3: audio.playSFX(SoundType::CLEAR_TRIPLE); break;
            case 4: audio.playSFX(SoundType::CLEAR_TETRIS); break;
        }
        calculateScore(linesCleared);
    }
    spawnPiece();
}

void Game::calculateScore(int linesCleared) {
    totalLines += linesCleared;

    // Diem nhan theo cap do.
    int points = 0;
    switch (linesCleared) {
        case 1: points = 100; break; // Single
        case 2: points = 300; break; // Double
        case 3: points = 500; break; // Triple
        case 4: points = 800; break; // Tetris!
    }
    score += points * level;

    // Tang cap do moi 10 dong, toc do roi toi da den 0.1s/o.
    level        = (totalLines / 10) + 1;
    fallInterval = std::max(0.1f, 1.0f - (level - 1) * 0.1f);
}

void Game::changeState(GameState newState) {
    if (state == newState) return;

    // Chuyen sang game over: tam dung BGM roi moi phat SFX game over.
    if (newState == GameState::GAME_OVER) {
        audio.pauseBGM();
        audio.playSFX(SoundType::GAME_OVER);
    }

    // Roi khoi game over: bat lai BGM.
    if (state == GameState::GAME_OVER && newState != GameState::GAME_OVER) {
        audio.resumeBGM();
    }

    state = newState;
}

bool Game::isMouseInRect(int mouseX, int mouseY, SDL_FRect rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}