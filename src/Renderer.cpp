#include "Renderer.h"
#include "Board.h"
#include "Tetromino.h"
#include <cstdio>
#include <iostream>

// Constructor: Khởi tạo renderer và tải các tài nguyên cần thiết (Texture,
// Font)
Renderer::Renderer(SDL_Renderer *renderer)
    : sdlRenderer(renderer), bgTexture(nullptr), blockTexture(nullptr),
      font(nullptr) {

  // Khởi tạo thư viện SDL3_ttf nếu chưa được khởi tạo trước đó
  if (!TTF_WasInit()) {
    if (!TTF_Init()) {
      std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
    }
  }

  // Tải hình nền từ file ảnh bằng SDL3_image
  bgTexture = IMG_LoadTexture(sdlRenderer, "assets/bg.png");
  if (!bgTexture) {
    std::cerr << "Failed to load bg.png: " << SDL_GetError() << std::endl;
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
  if (bgTexture)
    SDL_DestroyTexture(bgTexture);
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
  if (bgTexture) {
    SDL_FRect dest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
    SDL_RenderTexture(sdlRenderer, bgTexture, nullptr, &dest);
  }

  // 2. Vẽ viền bảng chơi (board) màu trắng
  SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
  SDL_FRect boardRect = {(float)BOARD_OFFSET_X, (float)BOARD_OFFSET_Y,
                         (float)(BOARD_WIDTH * CELL_SIZE),
                         (float)(BOARD_HEIGHT * CELL_SIZE)};
  SDL_RenderRect(sdlRenderer, &boardRect);

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
  int panelX = BOARD_OFFSET_X + BOARD_WIDTH * CELL_SIZE + 50;
  int panelY = BOARD_OFFSET_Y + 100;

  renderText("Next:", panelX, panelY - 40, {255, 255, 255, 255});

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

// Hàm tiện ích để vẽ một đoạn văn bản lên màn hình tại tọa độ (x,y)
void Renderer::renderText(const char *text, int x, int y, SDL_Color color) {
  if (!font)
    return;

  // Bước 1: Dùng SDL3_ttf render chữ ra bề mặt (Surface).
  // Tham số độ dài (length) truyền vào là 0 để hàm tự động đọc tới ký tự null
  // (\0).
  SDL_Surface *surface = TTF_RenderText_Solid(font, text, 0, color);
  if (surface) {
    // Bước 2: Chuyển đổi SDL_Surface thành SDL_Texture để GPU có thể vẽ nhanh
    // chóng
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    if (texture) {
      // Bước 3: Định dạng khung hình (chiều rộng và chiều cao khớp với kích
      // thước chữ)
      SDL_FRect destRect = {(float)x, (float)y, (float)surface->w,
                            (float)surface->h};
      SDL_RenderTexture(sdlRenderer, texture, nullptr, &destRect);
      SDL_DestroyTexture(texture); // Giải phóng Texture sau khi vẽ xong
    }
    SDL_DestroySurface(surface); // Giải phóng Surface
  }
}

void Renderer::drawUI(int score, int level, int lines) {
  int panelX = BOARD_OFFSET_X - 150;
  int panelY = BOARD_OFFSET_Y + 100;

  char buffer[64];

  snprintf(buffer, sizeof(buffer), "Score: %d", score);
  renderText(buffer, panelX, panelY, {255, 255, 255, 255});

  snprintf(buffer, sizeof(buffer), "Level: %d", level);
  renderText(buffer, panelX, panelY + 50, {255, 255, 255, 255});

  snprintf(buffer, sizeof(buffer), "Lines: %d", lines);
  renderText(buffer, panelX, panelY + 100, {255, 255, 255, 255});
}

void Renderer::drawScreen(GameState state) {
  switch (state) {
  case GameState::MENU:
    renderText("TETRIS", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50,
               {255, 255, 0, 255});
    renderText("Press ENTER to Start", WINDOW_WIDTH / 2 - 100,
               WINDOW_HEIGHT / 2 + 50, {255, 255, 255, 255});
    break;
  case GameState::PAUSED:
    renderText("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2,
               {255, 255, 255, 255});
    break;
  case GameState::GAME_OVER:
    renderText("GAME OVER", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 50,
               {255, 0, 0, 255});
    renderText("Press ENTER to Restart", WINDOW_WIDTH / 2 - 120,
               WINDOW_HEIGHT / 2 + 50, {255, 255, 255, 255});
    break;
  default:
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