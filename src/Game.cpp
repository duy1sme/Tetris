/*
 * Game.cpp
 * --------
 * Triển khai lớp Game — điều phối vòng lặp game, xử lý input,
 * cập nhật vật lý rơi mảnh và điều phối các subsystem.
 *
 * Vòng lặp chính (run()):
 *   ┌─────────────────────────────────────────┐
 *   │  Mỗi frame (~16.67ms ở 60 FPS):         │
 *   │  1. handleInput() — sự kiện SDL         │
 *   │  2. update(dt)    — vật lý rơi mảnh     │
 *   │  3. render()      — vẽ lên màn hình     │
 *   │  4. SDL_Delay()   — giới hạn 60 FPS     │
 *   └─────────────────────────────────────────┘
 *
 * Hệ thống điểm:
 *   Soft drop : +1 điểm/ô
 *   Hard drop : +2 điểm/ô
 *   Xóa hàng  : 100/300/500/800 × level (single/double/triple/tetris)
 *   Level tăng mỗi 10 hàng, tốc độ rơi tăng theo.
 */

#include "Game.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <iostream>

// ============================================================
// Constructor / Destructor
// ============================================================

// Khởi tạo tất cả con trỏ về nullptr và giá trị mặc định.
// Các subsystem thực sự được khởi tạo trong init().
Game::Game()
    : window(nullptr), sdlRenderer(nullptr), state(GameState::MENU),
      running(false), currentPiece(nullptr), nextPiece(nullptr), heldPiece(nullptr), canHold(true),
      renderer(nullptr), score(0), level(1), totalLines(0),
      fallTimer(0.0f), fallInterval(1.0f) {}

Game::~Game() {
    // Đảm bảo cleanup nếu run() kết thúc mà không gọi shutdown().
    if (running) {
        shutdown();
    }
}

// ============================================================
// init() — Khởi tạo toàn bộ hệ thống
// ============================================================

bool Game::init() {
    // Chỉ init VIDEO ở đây.
    // SDL_INIT_AUDIO được MIX_CreateMixerDevice() trong AudioManager::init()
    // tự xử lý nội bộ theo đúng API SDL3_mixer 3.2.
    if (!SDL_Init(SDL_INIT_VIDEO)) {     
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Tạo cửa sổ game với kích thước được định nghĩa trong GameState.h.
    window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Tạo renderer gắn với cửa sổ (dùng hardware acceleration nếu có).
    sdlRenderer = SDL_CreateRenderer(window, NULL);
    if (!sdlRenderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Bật alpha blending để vẽ bóng mờ (ghost piece) với độ trong suốt.
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    // Khởi tạo Renderer (load texture, font).
    renderer = new Renderer(sdlRenderer);

    // Khởi tạo âm thanh; nếu thất bại game vẫn chạy nhưng không có âm thanh.
    if (!audio.init()) {
        std::cerr << "Audio init failed: " << SDL_GetError() << std::endl;
    } else {
        audio.playBGM(); // Bắt đầu nhạc nền ngay sau khi vào menu.
    }

    running = true;
    state   = GameState::MENU;
    return true;
}

// ============================================================
// run() — Vòng lặp chính (Game Loop)
// ============================================================

void Game::run() {
    Uint64 lastTime = SDL_GetTicks();

    // Giới hạn 60 FPS để ổn định deltaTime và giảm tải CPU.
    const float TARGET_FPS     = 60.0f;
    const float FRAME_DURATION = 1000.0f / TARGET_FPS; // ≈16.67ms/frame

    while (running) {
        Uint64 currentTime = SDL_GetTicks();

        // deltaTime (giây) = thời gian từ frame trước đến frame này.
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Giới hạn deltaTime tối đa để tránh "spiral of death":
        // nếu frame kéo dài > 100ms (ví dụ khi debug/lag), mảnh sẽ không
        // rơi quá nhiều ô trong một bước cập nhật.
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        handleInput();      // Bước 1: xử lý sự kiện SDL
        update(deltaTime);  // Bước 2: cập nhật vật lý
        render();           // Bước 3: vẽ frame

        // Tính thời gian còn lại trong frame và delay để đạt đúng 60 FPS.
        Uint64 frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < (Uint64)FRAME_DURATION) {
            SDL_Delay((Uint32)(FRAME_DURATION - frameTime));
        }
    }
}

// ============================================================
// shutdown() — Giải phóng tài nguyên
// ============================================================

void Game::shutdown() {
    // Hủy heap objects theo thứ tự phụ thuộc.
    delete currentPiece; currentPiece = nullptr;
    delete nextPiece;    nextPiece    = nullptr;
    delete heldPiece;    heldPiece    = nullptr;
    delete renderer;     renderer     = nullptr;

    // Tắt âm thanh.
    audio.shutdown();

    // Hủy SDL objects.
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// ============================================================
// handleInput() — Xử lý sự kiện SDL
// ============================================================

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        // Cho AudioManager tự xử lý click chuột → phát tiếng button.
        audio.handleEvent(event);

        // ── Thoát ────────────────────────────────────────────────────
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        // ── Click chuột trái ─────────────────────────────────────────
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
            const float mouseX = event.button.x;  
            const float mouseY = event.button.y;

            // Nhấn "Play" trên menu → sang tutorial.
            if (state == GameState::MENU && isMouseInRect(mouseX, mouseY, btnPlay)) {
                changeState(GameState::TUTORIAL);
            }
            // Nhấn click bất kỳ ở tutorial → bắt đầu ván mới.
            else if (state == GameState::TUTORIAL) {
                board.reset();
                score = 0;
                level = 1;
                totalLines = 0;
                fallInterval = 1.0f;
                delete currentPiece;
                currentPiece = nullptr;
                delete nextPiece;
                nextPiece = nullptr;
                delete heldPiece;
                heldPiece = nullptr;
                canHold = true;
                spawnPiece();
                changeState(GameState::PLAYING);
            }

            // Nhấn nút trong popup Game Over.
            if (state == GameState::GAME_OVER) {
                const float popupX = (float)(WINDOW_WIDTH / 2 - 210);
                const float popupY = (float)(WINDOW_HEIGHT / 2 - 200);

                // Nút xoay (chơi lại) — bên phải popup.
                SDL_FRect btnRestart = {popupX + 174.0f, popupY + 308.0f, 72.0f, 72.0f};
                // Nút home (về menu) — bên trái popup.
                SDL_FRect btnHome = {popupX + 92.0f, popupY + 308.0f, 72.0f, 72.0f};

                if (isMouseInRect(mouseX, mouseY, btnRestart)) {
                // Chơi lại từ đầu.
                board.reset();
                score = 0;
                level = 1;
                totalLines = 0;
                fallInterval = 1.0f;
                delete currentPiece;
                currentPiece = nullptr;
                delete nextPiece;
                nextPiece = nullptr;
                delete heldPiece;
                heldPiece = nullptr;
                canHold = true;
                spawnPiece();
                changeState(GameState::PLAYING);
                } else if (isMouseInRect(mouseX, mouseY, btnHome)) {
                // Về menu chính.
                changeState(GameState::MENU);
                }
            }
            }

        // ── Phím bàn phím ─────────────────────────────────────────────
        else if (event.type == SDL_EVENT_KEY_DOWN) {

            // ESC: chuyển qua lại giữa PLAYING và PAUSED.
            if (event.key.key == SDLK_ESCAPE) {
                if (state == GameState::PLAYING)
                    changeState(GameState::PAUSED);
                else if (state == GameState::PAUSED)
                    changeState(GameState::PLAYING);
            }

            // ENTER ở menu → bắt đầu ván mới.
            if (state == GameState::MENU && event.key.key == SDLK_RETURN) {
                board.reset();
                score = 0; level = 1; totalLines = 0; fallInterval = 1.0f;
                delete currentPiece; currentPiece = nullptr;
                delete nextPiece;    nextPiece    = nullptr;
                delete heldPiece;    heldPiece    = nullptr;
                canHold = true;
                spawnPiece();
                changeState(GameState::PLAYING);
            }

            // ENTER ở Game Over → về menu.
            else if (state == GameState::GAME_OVER && event.key.key == SDLK_RETURN) {
                changeState(GameState::MENU);
            }

            // Các phím điều khiển chỉ hoạt động khi đang PLAYING.
            if (state != GameState::PLAYING || currentPiece == nullptr)
                continue;

            switch (event.key.key) {
            case SDLK_LEFT:
                // Di trái; hoàn tác nếu va chạm.
                currentPiece->moveLeft();
                if (!isValidPosition(*currentPiece)) currentPiece->moveRight();
                else audio.playSFX(SoundType::MOVE);
                break;

            case SDLK_RIGHT:
                // Di phải; hoàn tác nếu va chạm.
                currentPiece->moveRight();
                if (!isValidPosition(*currentPiece)) currentPiece->moveLeft();
                else audio.playSFX(SoundType::MOVE);
                break;

            case SDLK_DOWN:
                // Soft drop: rơi xuống 1 ô, +1 điểm nếu thành công.
                currentPiece->moveDown();
                if (!isValidPosition(*currentPiece)) {
                    currentPiece->moveUp(); // Hoàn tác — mảnh đã chạm đất.
                    lockCurrentPiece();
                } else {
                    score += 1;
                    audio.playSFX(SoundType::MOVE);
                }
                break;

            case SDLK_UP:
                // Xoay thuận chiều kim đồng hồ; hoàn tác nếu va chạm.
                currentPiece->rotateCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCCW();
                else audio.playSFX(SoundType::ROTATE);
                break;

            case SDLK_Z:
                // Xoay ngược chiều kim đồng hồ; hoàn tác nếu va chạm.
                currentPiece->rotateCCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCW();
                else audio.playSFX(SoundType::ROTATE);
                break;

            case SDLK_C:
                holdPiece();
                break;

            case SDLK_SPACE:
                hardDrop(); // Thả thẳng xuống — tách riêng để dễ đọc.
                break;
            }
        }
    }
}

// ============================================================
// hardDrop() — Thả mảnh thẳng xuống (SPACE)
// ============================================================
// Đếm số ô rơi để tính điểm (+2 điểm/ô), rồi khóa mảnh.

void Game::hardDrop() {
    if (!currentPiece) return;

    int cellsDropped = 0;
    // Rơi liên tục cho đến khi vị trí không còn hợp lệ.
    while (isValidPosition(*currentPiece)) {
        currentPiece->moveDown();
        cellsDropped++;
    }
    // Hoàn tác bước cuối (bước đó đã rơi vào vị trí không hợp lệ).
    currentPiece->moveUp();
    cellsDropped--;

    score += cellsDropped * 2; // +2 điểm mỗi ô hard drop.
    lockCurrentPiece();
}

// ============================================================
// update() — Cập nhật vật lý (rơi tự động)
// ============================================================

void Game::update(float deltaTime) {
    // Chỉ cập nhật khi đang PLAYING.
    if (state != GameState::PLAYING) return;

    fallTimer += deltaTime;

    // Khi đủ thời gian → rơi xuống 1 ô.
    if (fallTimer >= fallInterval) {
        fallTimer = 0.0f;

        if (currentPiece) {
            currentPiece->moveDown();
            if (!isValidPosition(*currentPiece)) {
                currentPiece->moveUp(); // Hoàn tác — mảnh chạm đất.
                lockCurrentPiece();
            }
        }
    }
}

// ============================================================
// render() — Vẽ toàn bộ khung hình
// ============================================================

void Game::render() {
    renderer->clear(); // Xóa frame cũ (nền đen).

    // Vẽ bảng chơi khi đang PLAYING, PAUSED hoặc GAME_OVER.
    if (state == GameState::PLAYING ||
        state == GameState::PAUSED  ||
        state == GameState::GAME_OVER) {

        renderer->drawBoard(board); // Nền game + các ô đã khóa.

        // Chỉ vẽ mảnh đang rơi khi đang PLAYING.
        if (state == GameState::PLAYING && currentPiece) {
            // Bóng mờ vẽ trước để nằm dưới mảnh thật.
            renderer->drawGhostPiece(*currentPiece, currentPiece->getGhostY(board));
            renderer->drawTetromino(*currentPiece);
        }

        if (nextPiece) renderer->drawNextPiece(*nextPiece);
        if (heldPiece) renderer->drawHeldPiece(*heldPiece);
        renderer->drawUI(score, level, totalLines);
    }

    // Vẽ lớp phủ theo trạng thái hiện tại (menu, pause, game over).
    if (state != GameState::PLAYING) {
        renderer->drawScreen(state, score, level, totalLines);
    }

    renderer->present(); // Hiển thị frame lên màn hình.
}

// ============================================================
// spawnPiece() — Tạo mảnh mới
// ============================================================

void Game::spawnPiece() {
    // Lần đầu chạy (chưa có nextPiece) → tạo mới.
    if (nextPiece == nullptr) {
        nextPiece = new Tetromino(Tetromino::createRandom());
    }

    canHold = true;

    // currentPiece ← nextPiece cũ.
    delete currentPiece;
    currentPiece = nextPiece;
    // Tạo nextPiece mới cho lần sau.
    nextPiece = new Tetromino(Tetromino::createRandom());

    // Nếu vị trí xuất hiện đã bị chặn → game over.
    if (!isValidPosition(*currentPiece)) {
        changeState(GameState::GAME_OVER);
    }
}

// ============================================================
// holdPiece() — Giữ mảnh hiện tại (C)
// ============================================================

void Game::holdPiece() {
    if (!canHold || !currentPiece) return;

    if (heldPiece == nullptr) {
        heldPiece = new Tetromino(currentPiece->getType());
        delete currentPiece;
        currentPiece = nullptr;
        spawnPiece();
    } else {
        TetrominoType currentType = currentPiece->getType();
        TetrominoType heldType = heldPiece->getType();
        
        delete currentPiece;
        currentPiece = new Tetromino(heldType);
        
        delete heldPiece;
        heldPiece = new Tetromino(currentType);

        if (!isValidPosition(*currentPiece)) {
            changeState(GameState::GAME_OVER);
        }
    }

    canHold = false;
    audio.playSFX(SoundType::MOVE); // Dùng âm thanh move tạm cho hold
}

// ============================================================
// isValidPosition() — Kiểm tra va chạm
// ============================================================
// Kiểm tra tất cả các ô của mảnh có nằm trong bảng và không
// chồng lên ô đã khóa không.

bool Game::isValidPosition(const Tetromino& piece) const {
    for (int r = 0; r < TETROMINO_SIZE; ++r) {
        for (int c = 0; c < TETROMINO_SIZE; ++c) {
            if (!piece.isCellFilled(c, r)) continue;

            int boardX = piece.x + c;
            int boardY = piece.y + r;

            // Kiểm tra biên trái, phải và dưới.
            if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT)
                return false;

            // Cho phép boardY âm (mảnh xuất hiện một phần ở trên bảng).
            if (boardY >= 0 && !board.isCellEmpty(boardX, boardY))
                return false;
        }
    }
    return true;
}

// ============================================================
// lockCurrentPiece() — Khóa mảnh, xóa hàng, tính điểm
// ============================================================

void Game::lockCurrentPiece() {
    if (!currentPiece) return;

    board.lockPiece(*currentPiece);     // Ghi mảnh vào lưới.
    audio.playSFX(SoundType::LOCK);     // Âm thanh chạm đất.

    int linesCleared = board.clearLines();
    if (linesCleared > 0) {
        // Phát SFX tương ứng số hàng xóa được.
        switch (linesCleared) {
            case 1: audio.playSFX(SoundType::CLEAR_SINGLE); break;
            case 2: audio.playSFX(SoundType::CLEAR_DOUBLE); break;
            case 3: audio.playSFX(SoundType::CLEAR_TRIPLE); break;
            case 4: audio.playSFX(SoundType::CLEAR_TETRIS); break;
        }
        calculateScore(linesCleared); // Cộng điểm và cập nhật level.
    }

    spawnPiece(); // Đưa mảnh tiếp theo vào bảng.
}

// ============================================================
// calculateScore() — Tính điểm và cập nhật cấp độ
// ============================================================
// Bảng điểm theo chuẩn Tetris Guideline (nhân thêm hệ số level):
//   1 hàng = 100 × level
//   2 hàng = 300 × level
//   3 hàng = 500 × level
//   4 hàng = 800 × level (Tetris!)

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

    // Tăng level mỗi 10 hàng; tốc độ rơi tối đa đạt 0.1s/ô ở level cao.
    level        = (totalLines / 10) + 1;
    fallInterval = std::max(0.1f, 1.0f - (level - 1) * 0.1f);
}

// ============================================================
// changeState() — Chuyển trạng thái game
// ============================================================
// Kèm theo logic audio:
//   → GAME_OVER : tạm dừng BGM, phát SFX game over.
//   Rời GAME_OVER: tiếp tục BGM.

void Game::changeState(GameState newState) {
    if (state == newState) return; // Không làm gì nếu đã ở trạng thái này.

    if (newState == GameState::GAME_OVER) {
        audio.pauseBGM();
        audio.playSFX(SoundType::GAME_OVER);
    }

    // Rời khỏi Game Over (về menu hoặc chơi lại) → bật lại BGM.
    if (state == GameState::GAME_OVER && newState != GameState::GAME_OVER) {
        audio.resumeBGM();
    }

    state = newState;
}

// ============================================================
// isMouseInRect() — Kiểm tra click trong vùng hình chữ nhật
// ============================================================

bool Game::isMouseInRect(float mouseX, float mouseY, SDL_FRect rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}