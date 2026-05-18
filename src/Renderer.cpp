/*
 * Renderer.cpp — Core rendering: constructor, utilities, game components.
 * Các màn hình UI (menu, settings, tutorial...) nằm trong RendererScreens.cpp.
 */
#include "Renderer.h"
#include "Board.h"
#include "Tetromino.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>

// ── Constructor ──────────────────────────────────────────────
Renderer::Renderer(SDL_Renderer* renderer)
    : sdlRenderer(renderer), fontLarge(nullptr), fontMedium(nullptr),
      fontSmall(nullptr), highscoresLoaded(false) {
    if (!TTF_WasInit()) {
        if (!TTF_Init())
            std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
    }
    const char* fontPath = "assets/images/pixel_font.ttf";
    fontLarge  = TTF_OpenFont(fontPath, 32);
    fontMedium = TTF_OpenFont(fontPath, 20);
    fontSmall  = TTF_OpenFont(fontPath, 14);
    if (!fontLarge || !fontMedium || !fontSmall)
        std::cerr << "Font load failed: " << SDL_GetError() << std::endl;
}

Renderer::~Renderer() {
    if (fontLarge)  TTF_CloseFont(fontLarge);
    if (fontMedium) TTF_CloseFont(fontMedium);
    if (fontSmall)  TTF_CloseFont(fontSmall);
    TTF_Quit();
}

// ── Frame lifecycle ──────────────────────────────────────────
void Renderer::clear() {
    SDL_SetRenderDrawColor(sdlRenderer, 10, 10, 26, 255);
    SDL_RenderClear(sdlRenderer);
}
void Renderer::present() { SDL_RenderPresent(sdlRenderer); }

// ── Color palette ────────────────────────────────────────────
SDL_Color Renderer::getTetrominoColor(TetrominoType type) {
    switch (type) {
    case TetrominoType::I: return {0,200,255,255};
    case TetrominoType::O: return {255,215,0,255};
    case TetrominoType::T: return {153,50,204,255};
    case TetrominoType::S: return {50,205,50,255};
    case TetrominoType::Z: return {220,20,60,255};
    case TetrominoType::J: return {30,144,255,255};
    case TetrominoType::L: return {255,140,0,255};
    default: return {255,255,255,255};
    }
}

// ── Text rendering ───────────────────────────────────────────
void Renderer::renderText(TTF_Font* f, const char* text, int x, int y, SDL_Color color) {
    if (!f) return;
    SDL_Surface* s = TTF_RenderText_Solid(f, text, 0, color);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer, s);
    if (t) {
        SDL_FRect d = {(float)x,(float)y,(float)s->w,(float)s->h};
        SDL_RenderTexture(sdlRenderer, t, nullptr, &d);
        SDL_DestroyTexture(t);
    }
    SDL_DestroySurface(s);
}

void Renderer::renderTextCentered(TTF_Font* f, const char* text, int cx, int cy, SDL_Color color) {
    if (!f) return;
    SDL_Surface* s = TTF_RenderText_Solid(f, text, 0, color);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(sdlRenderer, s);
    if (t) {
        SDL_FRect d = {(float)(cx-s->w/2),(float)(cy-s->h/2),(float)s->w,(float)s->h};
        SDL_RenderTexture(sdlRenderer, t, nullptr, &d);
        SDL_DestroyTexture(t);
    }
    SDL_DestroySurface(s);
}

int Renderer::getTextWidth(TTF_Font* f, const char* text) {
    if (!f) return 0;
    SDL_Surface* s = TTF_RenderText_Solid(f, text, 0, {255,255,255,255});
    if (!s) return 0;
    int w = s->w;
    SDL_DestroySurface(s);
    return w;
}

// ── drawBlock3D — beveled 3D block ──────────────────────────
void Renderer::drawBlock3D(float x, float y, float sz, SDL_Color c) {
    // Base fill
    SDL_FRect base = {x, y, sz, sz};
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(sdlRenderer, &base);

    // Highlight (top-left edges) — brighter
    uint8_t hr = (uint8_t)std::min(255, c.r + 60);
    uint8_t hg = (uint8_t)std::min(255, c.g + 60);
    uint8_t hb = (uint8_t)std::min(255, c.b + 60);
    SDL_SetRenderDrawColor(sdlRenderer, hr, hg, hb, 255);
    float bev = sz * 0.15f;
    SDL_FRect top = {x, y, sz, bev};
    SDL_FRect left = {x, y, bev, sz};
    SDL_RenderFillRect(sdlRenderer, &top);
    SDL_RenderFillRect(sdlRenderer, &left);

    // Shadow (bottom-right edges) — darker
    uint8_t sr = (uint8_t)(c.r * 0.45f);
    uint8_t sg = (uint8_t)(c.g * 0.45f);
    uint8_t sb = (uint8_t)(c.b * 0.45f);
    SDL_SetRenderDrawColor(sdlRenderer, sr, sg, sb, 255);
    SDL_FRect bot = {x, y + sz - bev, sz, bev};
    SDL_FRect right = {x + sz - bev, y, bev, sz};
    SDL_RenderFillRect(sdlRenderer, &bot);
    SDL_RenderFillRect(sdlRenderer, &right);

    // Inner face — slightly brighter than base
    uint8_t ir = (uint8_t)std::min(255, c.r + 15);
    uint8_t ig = (uint8_t)std::min(255, c.g + 15);
    uint8_t ib = (uint8_t)std::min(255, c.b + 15);
    SDL_SetRenderDrawColor(sdlRenderer, ir, ig, ib, c.a);
    SDL_FRect inner = {x+bev, y+bev, sz-2*bev, sz-2*bev};
    SDL_RenderFillRect(sdlRenderer, &inner);

    // Outer border
    SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 30, 255);
    SDL_RenderRect(sdlRenderer, &base);
}

// ── drawPanel ────────────────────────────────────────────────
void Renderer::drawPanel(float x, float y, float w, float h,
                          SDL_Color bg, SDL_Color border) {
    SDL_FRect r = {x, y, w, h};
    SDL_SetRenderDrawColor(sdlRenderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(sdlRenderer, &r);
    SDL_SetRenderDrawColor(sdlRenderer, border.r, border.g, border.b, border.a);
    SDL_RenderRect(sdlRenderer, &r);
    SDL_FRect r2 = {x+1, y+1, w-2, h-2};
    SDL_RenderRect(sdlRenderer, &r2);
}

// ── drawButton ───────────────────────────────────────────────
void Renderer::drawButton(const char* text, SDL_FRect rect, bool hovered, SDL_Color accent) {
    SDL_Color bg = hovered ? accent : SDL_Color{30, 30, 60, 255};
    SDL_Color border = hovered ? SDL_Color{255,255,255,255} : SDL_Color{80,80,120,255};
    drawPanel(rect.x, rect.y, rect.w, rect.h, bg, border);
    SDL_Color tc = {255, 255, 255, 255};
    renderTextCentered(fontMedium, text, (int)(rect.x+rect.w/2), (int)(rect.y+rect.h/2), tc);
}

// ── drawGameBackground — nền bảng chơi vẽ bằng code ─────────
void Renderer::drawGameBackground(int currentLevel) {
    // Sidebar panels
    SDL_Color panelBg = {20, 20, 45, 255};
    SDL_Color panelBorder = {50, 50, 90, 255};

    // Left panel
    drawPanel(20, 30, 280, 660, panelBg, panelBorder);
    // Right panel
    drawPanel(668, 30, 280, 660, panelBg, panelBorder);

    // Board background
    SDL_FRect boardBg = {(float)BOARD_OFFSET_X-2, (float)BOARD_OFFSET_Y-2,
                         (float)(BOARD_WIDTH*CELL_SIZE+4), (float)(BOARD_HEIGHT*CELL_SIZE+4)};
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 15, 255);
    SDL_RenderFillRect(sdlRenderer, &boardBg);

    // Grid lines
    SDL_SetRenderDrawColor(sdlRenderer, 25, 25, 50, 255);
    for (int c = 0; c <= BOARD_WIDTH; ++c) {
        float gx = (float)(BOARD_OFFSET_X + c * CELL_SIZE);
        SDL_RenderLine(sdlRenderer, gx, (float)BOARD_OFFSET_Y,
                       gx, (float)(BOARD_OFFSET_Y + BOARD_HEIGHT * CELL_SIZE));
    }
    for (int r = 0; r <= BOARD_HEIGHT; ++r) {
        float gy = (float)(BOARD_OFFSET_Y + r * CELL_SIZE);
        SDL_RenderLine(sdlRenderer, (float)BOARD_OFFSET_X, gy,
                       (float)(BOARD_OFFSET_X + BOARD_WIDTH * CELL_SIZE), gy);
    }

    // Board border with level theme color
    int li = std::max(0, std::min(currentLevel - 1, NUM_LEVELS - 1));
    // Find which config matches
    int ci = 0;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (currentLevel >= LEVEL_CONFIGS[i].startLevel) ci = i;
    }
    SDL_Color tc = {LEVEL_CONFIGS[ci].themeR, LEVEL_CONFIGS[ci].themeG, LEVEL_CONFIGS[ci].themeB, 255};
    SDL_SetRenderDrawColor(sdlRenderer, tc.r, tc.g, tc.b, 255);
    SDL_FRect borderR = {(float)BOARD_OFFSET_X-3, (float)BOARD_OFFSET_Y-3,
                         (float)(BOARD_WIDTH*CELL_SIZE+6), (float)(BOARD_HEIGHT*CELL_SIZE+6)};
    SDL_RenderRect(sdlRenderer, &borderR);
    SDL_FRect borderR2 = {(float)BOARD_OFFSET_X-4, (float)BOARD_OFFSET_Y-4,
                          (float)(BOARD_WIDTH*CELL_SIZE+8), (float)(BOARD_HEIGHT*CELL_SIZE+8)};
    SDL_RenderRect(sdlRenderer, &borderR2);

    // Panel labels
    SDL_Color white = {255,255,255,255};
    SDL_Color gray = {150,150,170,255};

    // Left: HOLD
    renderTextCentered(fontMedium, "HOLD", 160, 70, white);
    drawPanel(85, 90, 150, 130, {15,15,35,255}, panelBorder);

    // Left: SCORE / LEVEL / LINES
    renderTextCentered(fontSmall, "SCORE", 160, 280, gray);
    drawPanel(60, 300, 200, 50, {15,15,35,255}, panelBorder);
    renderTextCentered(fontSmall, "LEVEL", 160, 390, gray);
    drawPanel(60, 410, 200, 50, {15,15,35,255}, panelBorder);
    renderTextCentered(fontSmall, "LINES", 160, 500, gray);
    drawPanel(60, 520, 200, 50, {15,15,35,255}, panelBorder);

    // Right: NEXT
    renderTextCentered(fontMedium, "NEXT", 808, 70, white);
    drawPanel(733, 90, 150, 130, {15,15,35,255}, panelBorder);
}

// ── drawBoard ────────────────────────────────────────────────
void Renderer::drawBoard(const Board& board) {
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            TetrominoType type = board.getCellType(c, r);
            if (type != TetrominoType::NONE) {
                float px = (float)(BOARD_OFFSET_X + c * CELL_SIZE);
                float py = (float)(BOARD_OFFSET_Y + r * CELL_SIZE);
                drawBlock3D(px, py, (float)CELL_SIZE, getTetrominoColor(type));
            }
        }
    }
}

// ── drawTetromino ────────────────────────────────────────────
void Renderer::drawTetromino(const Tetromino& tetromino) {
    TetrominoType type = tetromino.getType();
    if (type == TetrominoType::NONE) return;
    SDL_Color color = getTetrominoColor(type);
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (tetromino.isCellFilled(c, r)) {
                float sx = (float)(BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE);
                float sy = (float)(BOARD_OFFSET_Y + (tetromino.y + r) * CELL_SIZE);
                if (sy >= BOARD_OFFSET_Y)
                    drawBlock3D(sx, sy, (float)CELL_SIZE, color);
            }
        }
    }
}

// ── drawGhostPiece ───────────────────────────────────────────
void Renderer::drawGhostPiece(const Tetromino& tetromino, int ghostY) {
    TetrominoType type = tetromino.getType();
    if (type == TetrominoType::NONE) return;
    SDL_Color color = getTetrominoColor(type);
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (tetromino.isCellFilled(c, r)) {
                float sx = (float)(BOARD_OFFSET_X + (tetromino.x + c) * CELL_SIZE);
                float sy = (float)(BOARD_OFFSET_Y + (ghostY + r) * CELL_SIZE);
                if (sy >= BOARD_OFFSET_Y) {
                    SDL_FRect rect = {sx, sy, (float)CELL_SIZE, (float)CELL_SIZE};
                    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 60);
                    SDL_RenderFillRect(sdlRenderer, &rect);
                    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 120);
                    SDL_RenderRect(sdlRenderer, &rect);
                    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
                }
            }
        }
    }
}

// ── drawNextPiece ────────────────────────────────────────────
void Renderer::drawNextPiece(const Tetromino& nextPiece) {
    int panelX = 745; int panelY = 105;
    TetrominoType type = nextPiece.getType();
    if (type == TetrominoType::NONE) return;
    SDL_Color color = getTetrominoColor(type);
    float bs = 28.0f;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            if (nextPiece.isCellFilled(c, r))
                drawBlock3D((float)(panelX + c * bs), (float)(panelY + r * bs), bs, color);
}

// ── drawHeldPiece ────────────────────────────────────────────
void Renderer::drawHeldPiece(const Tetromino& heldPiece) {
    int panelX = 97; int panelY = 105;
    TetrominoType type = heldPiece.getType();
    if (type == TetrominoType::NONE) return;
    SDL_Color color = getTetrominoColor(type);
    float bs = 28.0f;
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            if (heldPiece.isCellFilled(c, r))
                drawBlock3D((float)(panelX + c * bs), (float)(panelY + r * bs), bs, color);
}

// ── drawUI — score/level/lines ───────────────────────────────
void Renderer::drawUI(int score, int level, int lines) {
    SDL_Color white = {255,255,255,255};
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", score);
    renderTextCentered(fontMedium, buf, 160, 325, white);
    snprintf(buf, sizeof(buf), "%d", level);
    renderTextCentered(fontMedium, buf, 160, 435, white);
    snprintf(buf, sizeof(buf), "%d", lines);
    renderTextCentered(fontMedium, buf, 160, 545, white);
}

// ── loadHighscores ───────────────────────────────────────────
void Renderer::loadHighscores() {
    highscores.clear();
    std::ifstream file("highscores.txt");
    int sc;
    while (file >> sc) highscores.push_back(sc);
    std::sort(highscores.rbegin(), highscores.rend());
    if (highscores.size() > 5) highscores.resize(5);
}

// ── drawLineClearEffect ──────────────────────────────────────
void Renderer::drawLineClearEffect(const Board& board, int clearedRows[], int count) {
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 150);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < count; ++i) {
        int r = clearedRows[i];
        SDL_FRect rect = {(float)BOARD_OFFSET_X, (float)(BOARD_OFFSET_Y + r * CELL_SIZE),
                          (float)(BOARD_WIDTH * CELL_SIZE), (float)CELL_SIZE};
        SDL_RenderFillRect(sdlRenderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
    SDL_RenderPresent(sdlRenderer);
    SDL_Delay(50);
}