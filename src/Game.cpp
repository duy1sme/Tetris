#include "Game.h"
#include <algorithm>
#include <iostream>


Game::Game()
    : window(nullptr), sdlRenderer(nullptr), state(GameState::MENU),
      running(false), currentPiece(nullptr), nextPiece(nullptr),
      renderer(nullptr), score(0), level(1), totalLines(0), fallTimer(0.0f),
      fallInterval(1.0f) {}

bool Game::init() {
  // Khởi động SDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
    return false;
  }

  // Tạo cửa sổ
  window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!window) {
    std::cerr << "Window failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
  }

  // Tạo SDL Renderer
  sdlRenderer = SDL_CreateRenderer(window, NULL);
  if (!sdlRenderer) {
    std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return false;
  }

  // Tạo Renderer class
  renderer = new Renderer(sdlRenderer);

  // Khởi động audio
  audio.init();

  running = true;
  state = GameState::MENU;
  return true;
}

void Game::run() {
  Uint64 lastTime = SDL_GetTicks();

  while (running) {
    // Tính deltaTime
    Uint64 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    handleInput();
    update(deltaTime);
    render();
  }
}

void Game::shutdown() {
  delete currentPiece;
  delete nextPiece;
  delete renderer;

  audio.shutdown();

  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

Game::~Game() {
  // Đảm bảo cleanup nếu ai đó quên gọi shutdown()
  if (running) {
    shutdown();
  }
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        // ── MENU ──────────────────────────────────
        if (state == GameState::MENU) {
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx = (int)event.button.x;
                int my = (int)event.button.y;

                if (isMouseInRect(mx, my, btnPlay)) {
                    spawnPiece();
                    changeState(GameState::PLAYING);
                }
                // settings, help xử lý sau
            }
        }

        // ── PLAYING ───────────────────────────────
        else if (state == GameState::PLAYING) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    case SDL_SCANCODE_LEFT:
                        currentPiece->moveLeft();
                        if (!isValidPosition(*currentPiece)) currentPiece->moveRight();
                        break;
                    case SDL_SCANCODE_RIGHT:
                        currentPiece->moveRight();
                        if (!isValidPosition(*currentPiece)) currentPiece->moveLeft();
                        break;
                    case SDL_SCANCODE_DOWN:
                        currentPiece->moveDown();
                        if (!isValidPosition(*currentPiece)) currentPiece->moveUp();
                        break;
                    case SDL_SCANCODE_UP:
                    case SDL_SCANCODE_X:
                        currentPiece->rotateCW();
                        if (!isValidPosition(*currentPiece)) currentPiece->rotateCCW();
                        break;
                    case SDL_SCANCODE_Z:
                        currentPiece->rotateCCW();
                        if (!isValidPosition(*currentPiece)) currentPiece->rotateCW();
                        break;
                    case SDL_SCANCODE_SPACE:
                        hardDrop();
                        break;
                    case SDL_SCANCODE_P:
                    case SDL_SCANCODE_ESCAPE:
                        changeState(GameState::PAUSED);
                        break;
                }
            }
        }

        // ── PAUSED ────────────────────────────────
        else if (state == GameState::PAUSED) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_P ||
                    event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    changeState(GameState::PLAYING);
                }
            }
        }

        // ── GAME OVER ─────────────────────────────
        else if (state == GameState::GAME_OVER) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_RETURN) {
                    board.reset();
                    score = 0; level = 1; totalLines = 0;
                    spawnPiece();
                    changeState(GameState::PLAYING);
                }
            }
        }
    }
}
void Game::update(float deltaTime) {
  if (state != GameState::PLAYING)
    return;

  fallTimer += deltaTime;
  if (fallTimer >= fallInterval) {
    fallTimer = 0.0f;

    if (currentPiece) {
      currentPiece->moveDown();
      if (!isValidPosition(*currentPiece)) {
        currentPiece->y -= 1;
        lockCurrentPiece();
      }
    }
  }
}

void Game::render() {
  renderer->clear();

  if (state == GameState::PLAYING || state == GameState::GAME_OVER || state == GameState::PAUSED) {
    renderer->drawBoard(board);
    if (state == GameState::PLAYING && currentPiece) {
      renderer->drawGhostPiece(*currentPiece, currentPiece->getGhostY(board));
      renderer->drawTetromino(*currentPiece);
    }
    if (nextPiece) {
      renderer->drawNextPiece(*nextPiece);
    }
    renderer->drawUI(score, level, totalLines);
  }

  if (state != GameState::PLAYING) {
    renderer->drawScreen(state, score, level, totalLines);
  }

  renderer->present();
}

void Game::spawnPiece() {
    delete currentPiece;
    currentPiece = nextPiece ? nextPiece : new Tetromino(Tetromino::createRandom());
    nextPiece    = new Tetromino(Tetromino::createRandom());

    if (!isValidPosition(*currentPiece)) {
        changeState(GameState::GAME_OVER);
    }
}

bool Game::isValidPosition(const Tetromino &piece) const {
  for (int r = 0; r < TETROMINO_SIZE; ++r) {
    for (int c = 0; c < TETROMINO_SIZE; ++c) {
      if (piece.isCellFilled(c, r)) {
        int boardX = piece.x + c;
        int boardY = piece.y + r;

        if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT) {
          return false;
        }

        if (boardY >= 0 && !board.isCellEmpty(boardX, boardY)) {
          return false;
        }
      }
    }
  }
  return true;
}

void Game::lockCurrentPiece() {
  if (currentPiece) {
    board.lockPiece(*currentPiece);
    int linesCleared = board.clearLines();
    if (linesCleared > 0) {
      calculateScore(linesCleared);
    }
    spawnPiece();
  }
}

void Game::calculateScore(int linesCleared) {
  totalLines += linesCleared;

  int points = 0;
  switch (linesCleared) {
  case 1:
    points = 100;
    break;
  case 2:
    points = 300;
    break;
  case 3:
    points = 500;
    break;
  case 4:
    points = 800;
    break;
  }
  score += points * level;

  level = (totalLines / 10) + 1;
  fallInterval = std::max(0.1f, 1.0f - (level - 1) * 0.1f);
}

void Game::changeState(GameState newState) { state = newState; }

bool Game::isMouseInRect(int mouseX, int mouseY, SDL_FRect rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w
        && mouseY >= rect.y && mouseY <= rect.y + rect.h;
}

void Game::hardDrop() {
    while (isValidPosition(*currentPiece)) {
        currentPiece->moveDown();
    }
    currentPiece->moveUp();
    lockCurrentPiece();
}

