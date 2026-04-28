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
    } else if (event.type == SDL_EVENT_KEY_DOWN) {

      if (event.key.key == SDLK_ESCAPE) {
        if (state == GameState::PLAYING)
          changeState(GameState::PAUSED);
        else if (state == GameState::PAUSED)
          changeState(GameState::PLAYING);
      }

      if (state == GameState::MENU && event.key.key == SDLK_RETURN) {
        board.reset();
        score = 0;
        level = 1;
        totalLines = 0;
        fallInterval = 1.0f;
        delete currentPiece;
        currentPiece = nullptr;
        delete nextPiece;
        nextPiece = nullptr;
        spawnPiece();
        changeState(GameState::PLAYING);
      } else if (state == GameState::GAME_OVER &&
                 event.key.key == SDLK_RETURN) {
        changeState(GameState::MENU);
      }

      if (state != GameState::PLAYING)
        continue;
      if (currentPiece == nullptr)
        continue;

      switch (event.key.key) {
      case SDLK_LEFT:
        currentPiece->moveLeft();
        if (!isValidPosition(*currentPiece))
          currentPiece->moveRight();
        break;
      case SDLK_RIGHT:
        currentPiece->moveRight();
        if (!isValidPosition(*currentPiece))
          currentPiece->moveLeft();
        break;
      case SDLK_DOWN:
        currentPiece->moveDown();
        if (!isValidPosition(*currentPiece)) {
          currentPiece->y -= 1;
          lockCurrentPiece();
        } else {
          score += 1; // Soft drop points
        }
        break;
      case SDLK_UP:
      case SDLK_X:
        currentPiece->rotateCW();
        if (!isValidPosition(*currentPiece))
          currentPiece->rotateCCW();
        break;
      case SDLK_Z:
        currentPiece->rotateCCW();
        if (!isValidPosition(*currentPiece))
          currentPiece->rotateCW();
        break;
      case SDLK_SPACE:
        while (isValidPosition(*currentPiece)) {
          currentPiece->moveDown();
          score += 2;
        }
        currentPiece->y -= 1;
        score -= 2;
        lockCurrentPiece();
        break;
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
  if (nextPiece == nullptr) {
    nextPiece = new Tetromino(Tetromino::createRandom());
  }

  delete currentPiece;
  currentPiece = nextPiece;
  nextPiece = new Tetromino(Tetromino::createRandom());

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