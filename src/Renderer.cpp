/*
 * Renderer.cpp
 * ------------
 * Triển khai Renderer — load tài nguyên và vẽ toàn bộ giao diện game.
 *
 * Kỹ thuật vẽ block:
 *   Thay vì dùng nhiều texture màu khác nhau, chỉ cần một sprite
 *   block_green.png. Trước mỗi lần vẽ, gọi SDL_SetTextureColorMod()
 *   để tô màu tương ứng loại mảnh Tetromino.
 *
 * Kỹ thuật vẽ text:
 *   Mỗi lần vẽ text: render TTF_Surface → tạo texture tạm → vẽ → hủy.
 *   Không cache texture chữ vì số điểm thay đổi liên tục.
 *
 * Hệ thống highscore:
 *   File "highscores.txt" lưu mỗi điểm trên một dòng.
 *   Mỗi khi GAME_OVER, điểm hiện tại được gộp vào, sắp xếp giảm dần,
 *   giữ tối đa 5 điểm rồi ghi lại file.
 */

#include "Renderer.h"
#include "Board.h"
#include "Tetromino.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

// ============================================================
// Constructor — Load tất cả tài nguyên
// ============================================================

Renderer::Renderer(SDL_Renderer* renderer)
    : sdlRenderer(renderer), mainMenuTexture(nullptr),
      gameScreenTexture(nullptr), highScoreTexture(nullptr),
      blockTexture(nullptr), font(nullptr), highscoresLoaded(false) {

    // Khởi tạo SDL_ttf nếu chưa có (an toàn khi gọi nhiều lần).
    if (!TTF_WasInit()) {
        if (!TTF_Init()) {
            std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        }
    }

    // Load texture nền màn hình chính.
    mainMenuTexture = IMG_LoadTexture(sdlRenderer, "assets/images/main_menu.png");
    if (!mainMenuTexture) {
        std::cerr << "Failed to load main_menu.png: " << SDL_GetError() << std::endl;
    }

    // Load texture nền bảng chơi.
    gameScreenTexture = IMG_LoadTexture(sdlRenderer, "assets/images/gamescreen.png");
    if (!gameScreenTexture) {
        std::cerr << "Failed to load gamescreen.png: " << SDL_GetError() << std::endl;
    }

    // Load texture popup game over / bảng điểm.
    highScoreTexture = IMG_LoadTexture(sdlRenderer, "assets/images/highscore.png");
    if (!highScoreTexture) {
        std::cerr << "Failed to load highscore.png: " << SDL_GetError() << std::endl;
    }

    // Load sprite block — màu sẽ được thay đổi runtime bằng ColorMod.
    blockTexture = IMG_LoadTexture(sdlRenderer, "assets/images/block_green.png");
    if (!blockTexture) {
        std::cerr << "Failed to load block_green.png: " << SDL_GetError() << std::endl;
    }

    // Load font TTF cỡ 24px cho giao diện.
    font = TTF_OpenFont("assets/images/font.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font.ttf: " << SDL_GetError() << std::endl;
    }
}

// ============================================================
// Destructor — Giải phóng tài nguyên
// ============================================================

Renderer::~Renderer() {
    if (mainMenuTexture)  SDL_DestroyTexture(mainMenuTexture);
    if (gameScreenTexture)SDL_DestroyTexture(gameScreenTexture);
    if (highScoreTexture) SDL_DestroyTexture(highScoreTexture);
    if (blockTexture)     SDL_DestroyTexture(blockTexture);
    if (font)             TTF_CloseFont(font);
    TTF_Quit();
}

// ============================================================
// Frame Lifecycle
// ============================================================

// Xóa toàn bộ frame cũ bằng màu đen trước khi bắt đầu vẽ frame mới.
void Renderer::clear() {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
}

// Hiển thị frame đã vẽ lên màn hình (swap back buffer → front buffer).
void Renderer::present() {
    SDL_RenderPresent(sdlRenderer);
}

// ============================================================
// Bảng màu Tetromino — theo chuẩn Tetris Guideline
// ============================================================

SDL_Color Renderer::getTetrominoColor(TetrominoType type) {
    switch (type) {
    case TetrominoType::I: return {0,   255, 255, 255}; // Xanh lơ (cyan)
    case TetrominoType::O: return {255, 255, 0,   255}; // Vàng
    case TetrominoType::T: return {128, 0,   128, 255}; // Tím
    case TetrominoType::S: return {0,   255, 0,   255}; // Xanh lá
    case TetrominoType::Z: return {255, 0,   0,   255}; // Đỏ
    case TetrominoType::J: return {0,   0,   255, 255}; // Xanh dương
    case TetrominoType::L: return {255, 165, 0,   255}; // Cam
    default:               return {255, 255, 255, 255}; // Trắng (fallback)
    }
}

// ============================================================
// drawBoard() — Vẽ nền bảng chơi và các ô đã khóa
// ============================================================

void Renderer::drawBoard(const Board& board) {
    // Vẽ ảnh nền bảng chơi toàn màn hình.
    if (gameScreenTexture) {
        SDL_FRect dest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
        SDL_RenderTexture(sdlRenderer, gameScreenTexture, nullptr, &dest);
    }

    // Duyệt qua từng ô và vẽ nếu không trống.
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            TetrominoType type = board.getCellType(c, r);
            if (type != TetrominoType::NONE) {
                // Tính tọa độ pixel từ tọa độ lưới.
                SDL_FRect rect = {
                    (float)(BOARD_OFFSET_X + c * CELL_SIZE),
                    (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
                    (float)CELL_SIZE,
                    (float)CELL_SIZE
                };

                SDL_Color color = getTetrominoColor(type);
                if (blockTexture) {
                    // Tô màu sprite block theo loại mảnh.
                    SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
                    SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
                } else {
                    // Fallback: vẽ hình chữ nhật đặc nếu không có texture.
                    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
                    SDL_RenderFillRect(sdlRenderer, &rect);
                }

                // Viền mờ giữa các ô để dễ phân biệt.
                SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
                SDL_RenderRect(sdlRenderer, &rect);
            }
        }
    }
}

// ============================================================
// drawTetromino() — Vẽ mảnh đang rơi
// ============================================================

void Renderer::drawTetromino(const Tetromino& tetromino) {
    TetrominoType type = tetromino.getType();
    if (type == TetrominoType::NONE) return;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (tetromino.isCellFilled(c, r)) {
                int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
                int screenY = BOARD_OFFSET_Y + (tetromino.y + r) * CELL_SIZE;

                // Chỉ vẽ các ô nằm trong vùng hiển thị (không vẽ ô phía trên bảng).
                if (screenY >= BOARD_OFFSET_Y) {
                    SDL_FRect rect = {
                        (float)screenX, (float)screenY,
                        (float)CELL_SIZE, (float)CELL_SIZE
                    };

                    SDL_Color color = getTetrominoColor(type);
                    if (blockTexture) {
                        SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
                        SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
                    } else {
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

// ============================================================
// drawGhostPiece() — Vẽ bóng mờ tại vị trí hạ cánh dự kiến
// ============================================================
// ghostY là hàng Y mà mảnh sẽ chạm đất (tính bởi Tetromino::getGhostY).
// Bóng mờ được vẽ với alpha = 100/255 ≈ 39% để phân biệt với mảnh thật.

void Renderer::drawGhostPiece(const Tetromino& tetromino, int ghostY) {
    TetrominoType type = tetromino.getType();
    if (type == TetrominoType::NONE) return;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (tetromino.isCellFilled(c, r)) {
                int screenX = BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE;
                int screenY = BOARD_OFFSET_Y + (ghostY + r) * CELL_SIZE;

                if (screenY >= BOARD_OFFSET_Y) {
                    SDL_FRect rect = {
                        (float)screenX, (float)screenY,
                        (float)CELL_SIZE, (float)CELL_SIZE
                    };

                    SDL_Color color = getTetrominoColor(type);
                    if (blockTexture) {
                        SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
                        // Giảm alpha để tạo hiệu ứng trong suốt.
                        SDL_SetTextureAlphaMod(blockTexture, 100);
                        SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
                        // Khôi phục alpha về 255 cho các lần vẽ tiếp theo.
                        SDL_SetTextureAlphaMod(blockTexture, 255);
                    } else {
                        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 100);
                        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
                        SDL_RenderFillRect(sdlRenderer, &rect);
                        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
                    }

                    // Viền sáng hơn để phân biệt ghost với ô đã khóa.
                    SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
                    SDL_RenderRect(sdlRenderer, &rect);
                }
            }
        }
    }
}

// ============================================================
// drawNextPiece() — Vẽ mảnh tiếp theo ở panel bên phải
// ============================================================

void Renderer::drawNextPiece(const Tetromino& nextPiece) {
    // Tọa độ góc trên-trái của ô hiển thị "Next" trên panel phải.
    int panelX = 730;
    int panelY = 183;

    TetrominoType type = nextPiece.getType();
    if (type == TetrominoType::NONE) return;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (nextPiece.isCellFilled(c, r)) {
                SDL_FRect rect = {
                    (float)(panelX + c * CELL_SIZE),
                    (float)(panelY + r * CELL_SIZE),
                    (float)CELL_SIZE,
                    (float)CELL_SIZE
                };

                SDL_Color color = getTetrominoColor(type);
                if (blockTexture) {
                    SDL_SetTextureColorMod(blockTexture, color.r, color.g, color.b);
                    SDL_RenderTexture(sdlRenderer, blockTexture, nullptr, &rect);
                } else {
                    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
                    SDL_RenderFillRect(sdlRenderer, &rect);
                }

                SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
                SDL_RenderRect(sdlRenderer, &rect);
            }
        }
    }
}

// ============================================================
// renderText() / renderTextCentered() — Vẽ chữ với TTF
// ============================================================

// Vẽ text tại tọa độ (x, y) là góc trên-trái.
void Renderer::renderText(const char* text, int x, int y, SDL_Color color) {
    if (!font) return;

    // TTF_RenderText_Solid tạo surface 1-bit (nhanh, không anti-alias).
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, 0, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
        if (texture) {
            SDL_FRect destRect = {(float)x, (float)y,
                                  (float)surface->w, (float)surface->h};
            SDL_RenderTexture(sdlRenderer, texture, nullptr, &destRect);
            SDL_DestroyTexture(texture); // Hủy texture tạm ngay sau khi vẽ.
        }
        SDL_DestroySurface(surface);
    }
}

// Vẽ text căn giữa tại tâm (cx, cy).
void Renderer::renderTextCentered(const char* text, int cx, int cy, SDL_Color color) {
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, 0, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
        if (texture) {
            // Dịch về trái và lên trên một nửa kích thước text để căn giữa.
            SDL_FRect destRect = {
                (float)(cx - surface->w / 2),
                (float)(cy - surface->h / 2),
                (float)surface->w,
                (float)surface->h
            };
            SDL_RenderTexture(sdlRenderer, texture, nullptr, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_DestroySurface(surface);
    }
}

// ============================================================
// drawUI() — Vẽ thông tin điểm, level, số hàng
// ============================================================

void Renderer::drawUI(int score, int level, int lines) {
    int panelX = 177; // Tâm cột thông tin bên trái (pixel X).

    char buffer[64];

    snprintf(buffer, sizeof(buffer), "%d", score);
    renderTextCentered(buffer, panelX, 473, {255, 255, 255, 255});

    snprintf(buffer, sizeof(buffer), "%d", level);
    renderTextCentered(buffer, panelX, 545, {255, 255, 255, 255});

    snprintf(buffer, sizeof(buffer), "%d", lines);
    renderTextCentered(buffer, panelX, 617, {255, 255, 255, 255});
}

// ============================================================
// loadHighscores() — Đọc bảng điểm từ file
// ============================================================

void Renderer::loadHighscores() {
    highscores.clear();
    std::ifstream file("highscores.txt");
    int score;
    // Đọc từng dòng (mỗi dòng 1 số điểm).
    while (file >> score) {
        highscores.push_back(score);
    }
    // Sắp xếp giảm dần và giữ tối đa 5 điểm.
    std::sort(highscores.rbegin(), highscores.rend());
    if (highscores.size() > 5) {
        highscores.resize(5);
    }
}

// ============================================================
// drawScreen() — Vẽ màn hình theo trạng thái GameState
// ============================================================

void Renderer::drawScreen(GameState state, int currentScore,
                          int currentLevel, int currentLines) {
    SDL_FRect fullScreenDest = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};

    switch (state) {
    // ── MENU ──────────────────────────────────────────────────────────
    case GameState::MENU:
        highscoresLoaded = false; // Reset để GAME_OVER lần sau load lại.
        if (mainMenuTexture) {
            SDL_RenderTexture(sdlRenderer, mainMenuTexture, nullptr, &fullScreenDest);
        }
        break;

    // ── PAUSED ────────────────────────────────────────────────────────
    case GameState::PAUSED:
        highscoresLoaded = false;
        // Hiển thị chữ "PAUSED" overlay lên bảng chơi.
        renderText("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2,
                   {255, 255, 255, 255});
        break;

    // ── GAME OVER ─────────────────────────────────────────────────────
    case GameState::GAME_OVER: {
        // Load và lưu highscore chỉ một lần mỗi lần vào GAME_OVER.
        if (!highscoresLoaded) {
            loadHighscores();
            highscores.push_back(currentScore); // Thêm điểm ván vừa thua.
            std::sort(highscores.rbegin(), highscores.rend());
            if (highscores.size() > 5) {
                highscores.resize(5);
            }
            // Ghi lại file highscore.
            std::ofstream file("highscores.txt");
            for (int s : highscores) {
                file << s << "\n";
            }
            highscoresLoaded = true;
        }

        // Vẽ popup game over tại tâm màn hình.
        if (highScoreTexture) {
            SDL_FRect popupDest = {
                (float)(WINDOW_WIDTH / 2 - 210),
                (float)(WINDOW_HEIGHT / 2 - 200),
                420.0f, 400.0f
            };
            SDL_RenderTexture(sdlRenderer, highScoreTexture, nullptr, &popupDest);
        }

        int popupX = WINDOW_WIDTH / 2 - 210;
        int popupY = WINDOW_HEIGHT / 2 - 200;

        // Vẽ danh sách điểm cao nhất trong popup.
        if (highscores.empty()) {
            renderTextCentered("No scores yet!", popupX + 210, popupY + 180,
                               {200, 200, 200, 255});
        } else {
            bool highlighted = false;
            for (size_t i = 0; i < highscores.size(); ++i) {
                SDL_Color color = {255, 255, 255, 255};
                // Tô vàng điểm vừa đạt được (chỉ lần đầu tìm thấy).
                if (!highlighted && highscores[i] == currentScore) {
                    color     = {255, 255, 0, 255};
                    highlighted = true;
                }
                std::string scoreStr = std::to_string(highscores[i]);
                // Mỗi hàng điểm cách nhau ~29px.
                renderTextCentered(scoreStr.c_str(),
                                   popupX + 210, popupY + 134 + (int)i * 29,
                                   color);
            }
        }

        // Hướng dẫn người dùng.
        renderText("Press ENTER to Restart",
                   WINDOW_WIDTH / 2 - 140, WINDOW_HEIGHT / 2 + 220,
                   {255, 255, 255, 255});
        break;
    }

    default:
        highscoresLoaded = false;
        break;
    }
}

// ============================================================
// drawLineClearEffect() — Hiệu ứng nháy sáng khi xóa hàng
// ============================================================
// Tô trắng bán trong suốt lên các hàng vừa xóa rồi delay ngắn.
// Hàm này cần được gọi TRƯỚC khi Board::clearLines() thực sự xóa.

void Renderer::drawLineClearEffect(const Board& board,
                                   int clearedRows[], int count) {
    // Màu trắng bán trong suốt (alpha = 150/255 ≈ 59%).
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 150);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < count; ++i) {
        int r = clearedRows[i];
        SDL_FRect rect = {
            (float)BOARD_OFFSET_X,
            (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
            (float)(BOARD_WIDTH * CELL_SIZE),
            (float)CELL_SIZE
        };
        SDL_RenderFillRect(sdlRenderer, &rect);
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
}