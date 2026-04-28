#include "Renderer.h"
#include "Board.h"
#include "Tetromino.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// Constructor: Khởi tạo renderer và tải các tài nguyên cần thiết (Texture,
// Font)
Renderer::Renderer(SDL_Renderer *renderer)
    : sdlRenderer(renderer), mainMenuTexture(nullptr),
      gameScreenTexture(nullptr), highScoreTexture(nullptr),
      blockTexture(nullptr), font(nullptr), highscoresLoaded(false) {

  // Khởi tạo thư viện SDL3_ttf nếu chưa được khởi tạo trước đó
  if (!TTF_WasInit()) {
    if (!TTF_Init()) {
      std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
    }
  }

  mainMenuTexture = IMG_LoadTexture(sdlRenderer, "assets/main_menu.png");
  if (!mainMenuTexture) {
    std::cerr << "Failed to load main_menu.png: " << SDL_GetError()
              << std::endl;
  }

  gameScreenTexture = IMG_LoadTexture(sdlRenderer, "assets/gamescreen.png");
  if (!gameScreenTexture) {
    std::cerr << "Failed to load gamescreen.png: " << SDL_GetError()
              << std::endl;
  }

  highScoreTexture = IMG_LoadTexture(sdlRenderer, "assets/highscore.png");
  if (!highScoreTexture) {
    std::cerr << "Failed to load highscore.png: " << SDL_GetError()
              << std::endl;
  }

  // Tải hình ảnh của một khối block cơ bản (thường là ảnh grayscale để dễ phủ
  // màu)
  blockTexture = IMG_LoadTexture(sdlRenderer, "assets/block_green.png");
  if (!blockTexture) {
    std::cerr << "Failed to load block_green.png: " << SDL_GetError()
              << std::endl;
  }

  // Tải font chữ với kích thước 24pt bằng SDL3_ttf
  font = TTF_OpenFont("assets/font.ttf", 24);
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

// Hàm tiện ích: Trả về màu sắc (RGBA) tương ứng với từng loại mảnh Tetromino
SDL_Color Renderer::getTetrominoColor(TetrominoType type) {
  switch (type) {
  case TetrominoType::I:
    return {0, 255, 255, 255}; // Cyan (Xanh lơ)
  case TetrominoType::O:
    return {255, 255, 0, 255}; // Yellow (Vàng)
  case TetrominoType::T:
    return {128, 0, 128, 255}; // Purple (Tím)
  case TetrominoType::S:
    return {0, 255, 0, 255}; // Green (Xanh lá)
  case TetrominoType::Z:
    return {255, 0, 0, 255}; // Red (Đỏ)
  case TetrominoType::J:
    return {0, 0, 255, 255}; // Blue (Xanh dương)
  case TetrominoType::L:
    return {255, 165, 0, 255}; // Orange (Cam)
  default:
    return {255, 255, 255, 255}; // White (Trắng) dành cho các ô trống hoặc lỗi
  }
}

// Vẽ toàn bộ bảng chơi, bao gồm hình nền, viền và các khối đã bị khóa (locked)
void Renderer::drawBoard(const Board &board) {
  // 1. Vẽ background phủ toàn màn hình nếu đã load thành công
  if (gameScreenTexture) {
    SDL_FRect dest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
    SDL_RenderTexture(sdlRenderer, gameScreenTexture, nullptr, &dest);
  }

  // 2. Vẽ viền bảng chơi (board) màu trắng (Đã xóa vì ảnh nền đã có viền)
  // SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
  // SDL_FRect boardRect = {(float)BOARD_OFFSET_X, (float)BOARD_OFFSET_Y,
  //                        (float)(BOARD_WIDTH * CELL_SIZE),
  //                        (float)(BOARD_HEIGHT * CELL_SIZE)};
  // SDL_RenderRect(sdlRenderer, &boardRect);

  // 3. Duyệt qua từng ô trên bảng chơi để vẽ các khối đã nằm cố định
  for (int r = 0; r < BOARD_HEIGHT; ++r) {
    for (int c = 0; c < BOARD_WIDTH; ++c) {
      TetrominoType type = board.getCellType(c, r);
      if (type != TetrominoType::NONE) {
        // Tính toán tọa độ pixel trên màn hình
        SDL_FRect rect = {(float)(BOARD_OFFSET_X + c * CELL_SIZE),
                          (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
                          (float)CELL_SIZE, (float)CELL_SIZE};

        if (blockTexture) {
          // Nếu có ảnh blockTexture, ta sẽ áp dụng bộ lọc màu (Color Mod)
          // để nhuộm màu ảnh theo loại mảnh tương ứng
          SDL_Color color = getTetrominoColor(type);
          SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
          SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
        } else {
          SDL_Color color = getTetrominoColor(type);
          SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
          SDL_RenderFillRect(sdlRenderer, &rect);
        }

        // Vẽ lưới cho ô
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
        SDL_RenderRect(sdlRenderer, &rect);
      }
    }
  }
}

// Vẽ mảnh đang rơi (mảnh mà người chơi đang điều khiển)
void Renderer::drawTetromino(const Tetromino &tetromino) {
  TetrominoType type = tetromino.getType();
  if (type == TetrominoType::NONE)
    return;

  // Duyệt qua ma trận 4x4 của mảnh
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (tetromino.isCellFilled(c, r)) {
        // Tính toán vị trí thực tế trên màn hình dựa vào tọa độ x, y của mảnh
        int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
        int screenY = BOARD_OFFSET_Y + (tetromino.y + r) * CELL_SIZE;

        // Chỉ vẽ nếu khối nằm bên trong hoặc phía dưới mép trên của bảng chơi
        // (tránh vẽ lấn ra ngoài)
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

// Vẽ bóng mờ (Ghost Piece) hiển thị vị trí rơi dự kiến của mảnh hiện tại
void Renderer::drawGhostPiece(const Tetromino &tetromino, int ghostY) {
  TetrominoType type = tetromino.getType();
  if (type == TetrominoType::NONE)
    return;

  // Tương tự như mảnh đang rơi, duyệt ma trận 4x4
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      if (tetromino.isCellFilled(c, r)) {
        // Dùng tọa độ ghostY (vị trí thấp nhất có thể rơi) thay vì tọa độ y
        // hiện tại
        int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
        int screenY = BOARD_OFFSET_Y + (ghostY + r) * CELL_SIZE;

        if (screenY >= BOARD_OFFSET_Y) {
          SDL_FRect rect = {(float)screenX, (float)screenY, (float)CELL_SIZE,
                            (float)CELL_SIZE};

          if (blockTexture) {
            SDL_Color color = getTetrominoColor(type);
            SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);

            // Chỉnh Alpha = 100 để ảnh mờ đi (tạo hiệu ứng bóng mờ)
            SDL_SetTextureAlphaMod(blockTexture, 100);
            SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
            // Khôi phục Alpha về 255 (hoàn toàn đục) cho các lần vẽ sau
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
  int panelX = 177; // Center of the left boxes

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
          color = {255, 255, 0, 255}; // Highlight yellow
          highlighted = true;
        }
        std::string scoreStr = std::to_string(highscores[i]);
        // Box starts at 88, but top margin is ~12px. Rows start at ~100.
        // Row height is ~35px. First row center Y = 100 + 35/2 = 117.5
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
  SDL_Delay(50); // Delay nhẹ để có thể thấy hiệu ứng nhấp nháy
}