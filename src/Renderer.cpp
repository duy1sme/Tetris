#include "Renderer.h"
#include "Board.h"
#include "Tetromino.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// Tai texture va font dung cho cac man hinh.
Renderer::Renderer(SDL_Renderer *renderer)
    : sdlRenderer(renderer), mainMenuTexture(nullptr),
      gameScreenTexture(nullptr), highScoreTexture(nullptr),
      blockTexture(nullptr), font(nullptr), highscoresLoaded(false) {

  // TTF co the dung chung, chi khoi tao khi can.
  if (!TTF_WasInit()) {
    if (!TTF_Init()) {
      std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
    }
  }

  mainMenuTexture = IMG_LoadTexture(sdlRenderer, "assets/images/main_menu.png");
  if (!mainMenuTexture) {
    std::cerr << "Failed to load main_menu.png: " << SDL_GetError()
              << std::endl;
  }

  gameScreenTexture = IMG_LoadTexture(sdlRenderer, "assets/images/gamescreen.png");
  if (!gameScreenTexture) {
    std::cerr << "Failed to load gamescreen.png: " << SDL_GetError()
              << std::endl;
  }

  highScoreTexture = IMG_LoadTexture(sdlRenderer, "assets/images/highscore.png");
  if (!highScoreTexture) {
    std::cerr << "Failed to load highscore.png: " << SDL_GetError()
              << std::endl;
  }

  // Texture block goc se duoc doi mau luc ve.
  blockTexture = IMG_LoadTexture(sdlRenderer, "assets/images/block_green.png");
  if (!blockTexture) {
    std::cerr << "Failed to load block_green.png: " << SDL_GetError()
              << std::endl;
  }

  // Font cho giao dien.
  font = TTF_OpenFont("assets/images/font.ttf", 24);
  if (!font) {
    std::cerr << "Failed to load font.ttf: " << SDL_GetError() << std::endl;
  }
}

Renderer::~Renderer() {
  if (mainMenuTexture)
    SDL_DestroyTexture(mainMenuTexture);
  if (gameScreenTexture)
    SDL_DestroyTexture(gameScreenTexture);
  if (highScoreTexture)
    SDL_DestroyTexture(highScoreTexture);
  if (blockTexture)
    SDL_DestroyTexture(blockTexture);
  if (font)
    TTF_CloseFont(font);

  TTF_Quit();
}

void Renderer::clear() {
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
}

void Renderer::present() { SDL_RenderPresent(sdlRenderer); }

// Bang mau theo tung loai tetromino.
SDL_Color Renderer::getTetrominoColor(TetrominoType type) {
  switch (type) {
  case TetrominoType::I:
    return {0, 255, 255, 255}; // Xanh lo
  case TetrominoType::O:
    return {255, 255, 0, 255}; // Vang
  case TetrominoType::T:
    return {128, 0, 128, 255}; // Tim
  case TetrominoType::S:
    return {0, 255, 0, 255}; // Xanh la
  case TetrominoType::Z:
    return {255, 0, 0, 255}; // Red (Đỏ)
  case TetrominoType::J:
    return {0, 0, 255, 255}; // Xanh duong
  case TetrominoType::L:
    return {255, 165, 0, 255}; // Cam
  default:
    return {255, 255, 255, 255}; // Trang cho o trong hoac loi
  }
}

// Ve nen game va cac o da khoa tren bang choi.
void Renderer::drawBoard(const Board &board) {
  if (gameScreenTexture) {
    SDL_FRect dest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
    SDL_RenderTexture(sdlRenderer, gameScreenTexture, nullptr, &dest);
  }

  for (int r = 0; r < BOARD_HEIGHT; ++r) {
    for (int c = 0; c < BOARD_WIDTH; ++c) {
      TetrominoType type = board.getCellType(c, r);
      if (type != TetrominoType::NONE) {
        SDL_FRect rect = {(float)(BOARD_OFFSET_X + c * CELL_SIZE),
                          (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
                          (float)CELL_SIZE, (float)CELL_SIZE};

        if (blockTexture) {
          SDL_Color color = getTetrominoColor(type);
          SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
          SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
        } else {
          SDL_Color color = getTetrominoColor(type);
          SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
          SDL_RenderFillRect(sdlRenderer, &rect);
        }

        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
        SDL_RenderRect(sdlRenderer, &rect);
      }
    }
  }
}

// Ve manh dang duoc dieu khien.
void Renderer::drawTetromino(const Tetromino &tetromino) {
  TetrominoType type = tetromino.getType();
  if (type == TetrominoType::NONE)
    return;

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (tetromino.isCellFilled(c, r)) {
        int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
        int screenY = BOARD_OFFSET_Y + (tetromino.y + r) * CELL_SIZE;

        // Khong ve cac o nam tren vung hien thi cua bang choi.
        if (screenY >= BOARD_OFFSET_Y) {
          SDL_FRect rect = {(float)screenX, (float)screenY, (float)CELL_SIZE,
                            (float)CELL_SIZE};

          if (blockTexture) {
            SDL_Color color = getTetrominoColor(type);
            SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
            SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
          } else {
            SDL_Color color = getTetrominoColor(type);
            SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(sdlRenderer, &rect);
          }

          SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
          SDL_RenderRect(sdlRenderer, &rect);
        }
      }
    }
  }
}

// Ve bong mo tai vi tri ha canh du kien.
void Renderer::drawGhostPiece(const Tetromino &tetromino, int ghostY) {
  TetrominoType type = tetromino.getType();
  if (type == TetrominoType::NONE)
    return;

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (tetromino.isCellFilled(c, r)) {
        int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
        int screenY = BOARD_OFFSET_Y + (ghostY + r) * CELL_SIZE;

        if (screenY >= BOARD_OFFSET_Y) {
          SDL_FRect rect = {(float)screenX, (float)screenY, (float)CELL_SIZE,
                            (float)CELL_SIZE};

          if (blockTexture) {
            SDL_Color color = getTetrominoColor(type);
            SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);

            SDL_SetTextureAlphaMod(blockTexture, 100);
            SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
            // Khoi phuc do trong suot cho cac lan ve tiep theo.
            SDL_SetTextureAlphaMod(blockTexture, 255);
          } else {
            SDL_Color color = getTetrominoColor(type);
            SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 100);
            SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(sdlRenderer, &rect);
            SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
          }

          SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
          SDL_RenderRect(sdlRenderer, &rect);
        }
      }
    }
  }
}

void Renderer::drawNextPiece(const Tetromino &nextPiece) {
  int panelX = 730;
  int panelY = 183;

  TetrominoType type = nextPiece.getType();
  if (type == TetrominoType::NONE)
    return;

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (nextPiece.isCellFilled(c, r)) {
        SDL_FRect rect = {(float)(panelX + c * CELL_SIZE),
                          (float)(panelY + r * CELL_SIZE), (float)CELL_SIZE,
                          (float)CELL_SIZE};

        if (blockTexture) {
          SDL_Color color = getTetrominoColor(type);
          SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
          SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
        } else {
          SDL_Color color = getTetrominoColor(type);
          SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
          SDL_RenderFillRect(sdlRenderer, &rect);
        }

        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
        SDL_RenderRect(sdlRenderer, &rect);
      }
    }
  }
}

void Renderer::renderText(const char *text, int x, int y, SDL_Color color) {
  if (!font)
    return;

  SDL_Surface *surface = TTF_RenderText_Solid(font, text, 0, color);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    if (texture) {
      SDL_FRect destRect = {(float)x, (float)y, (float)surface->w,
                            (float)surface->h};
      SDL_RenderTexture(sdlRenderer, texture, nullptr, &destRect);
      SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
  }
}

void Renderer::renderTextCentered(const char *text, int cx, int cy,
                                  SDL_Color color) {
  if (!font)
    return;

  SDL_Surface *surface = TTF_RenderText_Solid(font, text, 0, color);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    if (texture) {
      SDL_FRect destRect = {(float)(cx - surface->w / 2),
                            (float)(cy - surface->h / 2), (float)surface->w,
                            (float)surface->h};
      SDL_RenderTexture(sdlRenderer, texture, nullptr, &destRect);
      SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
  }
}

void Renderer::drawUI(int score, int level, int lines) {
  int panelX = 177; // Tam cac o thong tin ben trai

  char buffer[64];

  snprintf(buffer, sizeof(buffer), "%d", score);
  renderTextCentered(buffer, panelX, 473, {255, 255, 255, 255});

  snprintf(buffer, sizeof(buffer), "%d", level);
  renderTextCentered(buffer, panelX, 545, {255, 255, 255, 255});

  snprintf(buffer, sizeof(buffer), "%d", lines);
  renderTextCentered(buffer, panelX, 617, {255, 255, 255, 255});
}

void Renderer::loadHighscores() {
  highscores.clear();
  std::ifstream file("highscores.txt");
  int score;
  while (file >> score) {
    highscores.push_back(score);
  }
  std::sort(highscores.rbegin(), highscores.rend());
  if (highscores.size() > 5) {
    highscores.resize(5);
  }
}

void Renderer::drawScreen(GameState state, int currentScore, int currentLevel,
                          int currentLines) {
  SDL_FRect fullScreenDest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};

  switch (state) {
  case GameState::MENU:
    highscoresLoaded = false;
    if (mainMenuTexture) {
      SDL_RenderTexture(sdlRenderer, mainMenuTexture, nullptr, &fullScreenDest);
    }
    // Chữ có thể bỏ đi vì ảnh đã vẽ rồi, hoặc nếu muốn thì bật lên
    // renderText("Press ENTER to Start", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT
    // / 2 + 50, {255, 255, 255, 255});
    break;
  case GameState::PAUSED:
    highscoresLoaded = false;
    renderText("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2,
               {255, 255, 255, 255});
    break;
  case GameState::GAME_OVER: {
    if (!highscoresLoaded) {
      loadHighscores();
      highscores.push_back(currentScore);
      std::sort(highscores.rbegin(), highscores.rend());
      if (highscores.size() > 5) {
        highscores.resize(5);
      }

      std::ofstream file("highscores.txt");
      for (int s : highscores) {
        file << s << "\n";
      }

      highscoresLoaded = true;
    }
    if (highScoreTexture) {
      SDL_FRect popupDest = {(float)(WINDOW_WIDTH / 2 - 210),
                             (float)(WINDOW_HEIGHT / 2 - 200), 420.0f, 400.0f};
      SDL_RenderTexture(sdlRenderer, highScoreTexture, nullptr, &popupDest);
    }

    // Tọa độ gốc của popup
    int popupX = WINDOW_WIDTH / 2 - 210;
    int popupY = WINDOW_HEIGHT / 2 - 200;

    // Vẽ Highscore History (Bảng vàng)
    if (highscores.empty()) {
      renderTextCentered("No scores yet!", popupX + 210, popupY + 180,
                         {200, 200, 200, 255});
    } else {
      bool highlighted = false;
      for (size_t i = 0; i < highscores.size(); ++i) {
        SDL_Color color = {255, 255, 255, 255};
        if (!highlighted && highscores[i] == currentScore) {
          color = {255, 255, 0, 255}; // To mau vang cho diem vua dat
          highlighted = true;
        }
        std::string scoreStr = std::to_string(highscores[i]);
        // Box starts at 88, but top margin is ~12px. Rows start at ~100.
        // Chieu cao moi dong xap xi 35px, canh theo tam tung dong.
        renderTextCentered(scoreStr.c_str(), popupX + 210,
                           popupY + 134 + i * 29, color);
      }
    }

    renderText("Press ENTER to Restart", WINDOW_WIDTH / 2 - 140,
               WINDOW_HEIGHT / 2 + 220, {255, 255, 255, 255});
    break;
  }
  default:
    highscoresLoaded = false;
    break;
  }
}

void Renderer::drawLineClearEffect(const Board &board, int clearedRows[],
                                   int count) {
  // Hiệu ứng nháy sáng trước khi xóa
  SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 150);
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

  for (int i = 0; i < count; ++i) {
    int r = clearedRows[i];
    SDL_FRect rect = {(float)BOARD_OFFSET_X,
                      (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
                      (float)(BOARD_WIDTH * CELL_SIZE), (float)CELL_SIZE};
    SDL_RenderFillRect(sdlRenderer, &rect);
  }

  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
  SDL_RenderPresent(sdlRenderer);
  SDL_Delay(50); // Tre nhe de nhin ro hieu ung nhap nhay
}