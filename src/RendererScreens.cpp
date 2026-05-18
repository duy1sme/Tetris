/*
 * RendererScreens.cpp — Vẽ các màn hình UI: Menu, Level Select, Settings,
 * Tutorial, Pause, Game Over. Tách riêng để giữ Renderer.cpp gọn.
 */
#include "Board.h"
#include "Renderer.h"
#include "Tetromino.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

// ── drawMenuScreen ───────────────────────────────────────────
void Renderer::drawMenuScreen(int hoveredBtn, float animTime) {
  // Animated falling blocks background
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
  for (int i = 0; i < 25; i++) {
    int bx = (i * 137 + 43) % WINDOW_WIDTH;
    float speed = 18.0f + (i % 5) * 12.0f;
    int by = (int)(animTime * speed + i * 97) % (WINDOW_HEIGHT + 60) - 40;
    TetrominoType bt = (TetrominoType)(i % 7);
    SDL_Color bc = getTetrominoColor(bt);
    SDL_SetRenderDrawColor(sdlRenderer, bc.r, bc.g, bc.b, 25);
    float bsz = 24.0f + (i % 3) * 8.0f;
    SDL_FRect br = {(float)bx, (float)by, bsz, bsz};
    SDL_RenderFillRect(sdlRenderer, &br);
  }
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);

  // Title — "TETRIS" using colored blocks
  const char *title = "TETRIS";
  int titleX = WINDOW_WIDTH / 2;
  int titleY = 140;

  renderTextCentered(fontLarge, title, titleX, titleY, {233, 69, 96, 255});

  // Subtitle
  renderTextCentered(fontSmall, "A Classic Block Puzzle", titleX, titleY + 45,
                     {120, 120, 150, 255});

  // Decorative line
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderLine(sdlRenderer, (float)(titleX - 150), (float)(titleY + 70),
                 (float)(titleX + 150), (float)(titleY + 70));

  // Menu buttons
  float btnW = 280, btnH = 50;
  float btnX = (WINDOW_WIDTH - btnW) / 2.0f;
  float startY = 280;
  float gap = 70;

  SDL_FRect btnPlay = {btnX, startY, btnW, btnH};
  SDL_FRect btnSettings = {btnX, startY + gap, btnW, btnH};
  SDL_FRect btnTutorial = {btnX, startY + gap * 2, btnW, btnH};

  drawButton("PLAY", btnPlay, hoveredBtn == 0, {46, 204, 113, 255});
  drawButton("SETTINGS", btnSettings, hoveredBtn == 1, {52, 152, 219, 255});
  drawButton("HOW TO PLAY", btnTutorial, hoveredBtn == 2, {155, 89, 182, 255});

  // Highscore display
  loadHighscores();
  if (!highscores.empty()) {
    char buf[64];
    snprintf(buf, sizeof(buf), "HIGH SCORE: %d", highscores[0]);
    renderTextCentered(fontSmall, buf, titleX, 560, {255, 215, 0, 255});
  }

  // Version/credit
  renderTextCentered(fontSmall, "Press ENTER to Play", titleX, 640,
                     {80, 80, 100, 255});
}

// ── drawLevelSelectScreen ────────────────────────────────────
void Renderer::drawLevelSelectScreen(int selectedLevel, int hoveredLevel) {
  renderTextCentered(fontLarge, "SELECT LEVEL", WINDOW_WIDTH / 2, 80,
                     {233, 69, 96, 255});

  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderLine(sdlRenderer, (float)(WINDOW_WIDTH / 2 - 150), 115,
                 (float)(WINDOW_WIDTH / 2 + 150), 115);

  float cardW = 240, cardH = 340;
  float totalW = cardW * NUM_LEVELS + 40 * (NUM_LEVELS - 1);
  float startX = (WINDOW_WIDTH - totalW) / 2.0f;
  float cardY = 160;

  for (int i = 0; i < NUM_LEVELS; i++) {
    float cx = startX + i * (cardW + 40);
    bool hov = (hoveredLevel == i);
    bool sel = (selectedLevel == i);
    SDL_Color theme = {LEVEL_CONFIGS[i].themeR, LEVEL_CONFIGS[i].themeG,
                       LEVEL_CONFIGS[i].themeB, 255};
    SDL_Color bg =
        hov || sel ? SDL_Color{30, 30, 60, 255} : SDL_Color{20, 20, 45, 255};
    SDL_Color border = hov || sel ? theme : SDL_Color{50, 50, 90, 255};

    drawPanel(cx, cardY, cardW, cardH, bg, border);
    if (hov || sel) {
      // Extra glow border
      SDL_SetRenderDrawColor(sdlRenderer, theme.r, theme.g, theme.b, 100);
      SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
      SDL_FRect gl = {cx - 2, cardY - 2, cardW + 4, cardH + 4};
      SDL_RenderRect(sdlRenderer, &gl);
      SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
    }

    int midX = (int)(cx + cardW / 2);
    renderTextCentered(fontMedium, LEVEL_CONFIGS[i].name, midX,
                       (int)(cardY + 40), theme);

    // Stars
    int stars = i + 1;
    char starBuf[8];
    for (int s = 0; s < 3; s++) {
      starBuf[s] = (s < stars) ? '*' : '-';
    }
    starBuf[3] = '\0';
    renderTextCentered(fontMedium, starBuf, midX, (int)(cardY + 85),
                       {255, 215, 0, 255});

    // Speed info
    char speedBuf[32];
    snprintf(speedBuf, sizeof(speedBuf), "Speed: %.1fs",
             LEVEL_CONFIGS[i].startFallInterval);
    renderTextCentered(fontSmall, speedBuf, midX, (int)(cardY + 130),
                       {150, 150, 170, 255});

    snprintf(speedBuf, sizeof(speedBuf), "Start Lv.%d",
             LEVEL_CONFIGS[i].startLevel);
    renderTextCentered(fontSmall, speedBuf, midX, (int)(cardY + 160),
                       {150, 150, 170, 255});

    // Decorative tetromino blocks
    TetrominoType deco[] = {TetrominoType::I, TetrominoType::T,
                            TetrominoType::S};
    SDL_Color dc = getTetrominoColor(deco[i]);
    float blockSz = 20.0f;
    float decoY = cardY + 200;
    // Draw a small tetromino shape
    if (i == 0) { // I piece
      for (int b = 0; b < 4; b++)
        drawBlock3D(cx + 50 + b * blockSz, decoY, blockSz, dc);
    } else if (i == 1) { // T piece
      drawBlock3D(cx + 70, decoY, blockSz, dc);
      drawBlock3D(cx + 90, decoY, blockSz, dc);
      drawBlock3D(cx + 110, decoY, blockSz, dc);
      drawBlock3D(cx + 90, decoY + blockSz, blockSz, dc);
    } else { // S piece
      drawBlock3D(cx + 90, decoY, blockSz, dc);
      drawBlock3D(cx + 110, decoY, blockSz, dc);
      drawBlock3D(cx + 70, decoY + blockSz, blockSz, dc);
      drawBlock3D(cx + 90, decoY + blockSz, blockSz, dc);
    }

    // Click hint
    renderTextCentered(fontSmall, "Click to Play", midX,
                       (int)(cardY + cardH - 30), {100, 100, 130, 255});
  }

  // Back button
  SDL_FRect btnBack = {(WINDOW_WIDTH - 200) / 2.0f, 570, 200, 45};
  drawButton("< BACK", btnBack, hoveredLevel == NUM_LEVELS);
}

// ── drawSettingsScreen ───────────────────────────────────────
void Renderer::drawSettingsScreen(float bgmVol, float sfxVol, bool bgmOn,
                                  bool sfxOn, int hoveredItem, int editingKey,
                                  const char *keyNames[6]) {
  renderTextCentered(fontLarge, "SETTINGS", WINDOW_WIDTH / 2, 60,
                     {233, 69, 96, 255});
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderLine(sdlRenderer, (float)(WINDOW_WIDTH / 2 - 150), 95,
                 (float)(WINDOW_WIDTH / 2 + 150), 95);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Color gray = {150, 150, 170, 255};
  SDL_Color accent = {233, 69, 96, 255};
  int leftX = 200;
  int rightX = 650;

  // ── AUDIO section ──
  renderText(fontMedium, "AUDIO", leftX, 130, accent);
  SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 90, 255);
  SDL_RenderLine(sdlRenderer, (float)leftX, 160, (float)(WINDOW_WIDTH - 200),
                 160);

  // BGM Volume
  int yBGM = 185;
  bool hovBGM = (hoveredItem == 0);
  renderText(fontSmall, "BGM Volume", leftX, yBGM, hovBGM ? white : gray);
  // Slider bar
  float sliderX = 450, sliderW = 250, sliderH = 16;
  SDL_FRect sliderBg = {sliderX, (float)(yBGM + 2), sliderW, sliderH};
  SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 60, 255);
  SDL_RenderFillRect(sdlRenderer, &sliderBg);
  SDL_FRect sliderFill = {sliderX, (float)(yBGM + 2), sliderW * bgmVol,
                          sliderH};
  SDL_SetRenderDrawColor(sdlRenderer, 46, 204, 113, 255);
  SDL_RenderFillRect(sdlRenderer, &sliderFill);
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderRect(sdlRenderer, &sliderBg);
  char volBuf[16];
  snprintf(volBuf, sizeof(volBuf), "%d%%", (int)(bgmVol * 100));
  renderText(fontSmall, volBuf, (int)(sliderX + sliderW + 15), yBGM, white);
  // ON/OFF
  const char *bgmState = bgmOn ? "[ON]" : "[OFF]";
  SDL_Color bgmCol =
      bgmOn ? SDL_Color{46, 204, 113, 255} : SDL_Color{231, 76, 60, 255};
  renderText(fontSmall, bgmState, rightX + 130, yBGM,
             hoveredItem == 1 ? white : bgmCol);

  // SFX Volume
  int ySFX = 230;
  bool hovSFX = (hoveredItem == 2);
  renderText(fontSmall, "SFX Volume", leftX, ySFX, hovSFX ? white : gray);
  SDL_FRect sliderBg2 = {sliderX, (float)(ySFX + 2), sliderW, sliderH};
  SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 60, 255);
  SDL_RenderFillRect(sdlRenderer, &sliderBg2);
  SDL_FRect sliderFill2 = {sliderX, (float)(ySFX + 2), sliderW * sfxVol,
                           sliderH};
  SDL_SetRenderDrawColor(sdlRenderer, 52, 152, 219, 255);
  SDL_RenderFillRect(sdlRenderer, &sliderFill2);
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderRect(sdlRenderer, &sliderBg2);
  snprintf(volBuf, sizeof(volBuf), "%d%%", (int)(sfxVol * 100));
  renderText(fontSmall, volBuf, (int)(sliderX + sliderW + 15), ySFX, white);
  const char *sfxState = sfxOn ? "[ON]" : "[OFF]";
  SDL_Color sfxCol =
      sfxOn ? SDL_Color{46, 204, 113, 255} : SDL_Color{231, 76, 60, 255};
  renderText(fontSmall, sfxState, rightX + 130, ySFX,
             hoveredItem == 3 ? white : sfxCol);

  // ── CONTROLS section ──
  renderText(fontMedium, "CONTROLS", leftX, 290, accent);
  SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 90, 255);
  SDL_RenderLine(sdlRenderer, (float)leftX, 320, (float)(WINDOW_WIDTH - 200),
                 320);

  const char *labels[] = {"Move Left", "Move Right", "Soft Drop",
                          "Hard Drop", "Rotate CW", "Rotate CCW", "Hold"};
  for (int i = 0; i < 7; i++) {
    int ky = 350 + i * 40;
    bool hov = (hoveredItem == 4 + i);
    bool editing = (editingKey == i);
    renderText(fontSmall, labels[i], leftX, ky, hov ? white : gray);

    // Key display box co giãn động dựa trên độ dài chữ
    const char *display = editing ? "Press a key..." : keyNames[i];
    int textW = getTextWidth(fontSmall, display);
    float boxW = std::max(180.0f, (float)(textW + 24)); // padding 12px mỗi bên, tối thiểu 180px
    SDL_FRect keyBox = {630.0f - boxW, (float)(ky - 2), boxW, 28.0f};

    SDL_Color keyBg =
        editing ? SDL_Color{80, 30, 30, 255} : SDL_Color{30, 30, 60, 255};
    SDL_Color keyBorder =
        editing ? accent : (hov ? white : SDL_Color{80, 80, 120, 255});
    drawPanel(keyBox.x, keyBox.y, keyBox.w, keyBox.h, keyBg, keyBorder);

    SDL_Color keyColor = editing ? accent : white;
    renderTextCentered(fontSmall, display, (int)(keyBox.x + keyBox.w / 2), ky + 10,
                       keyColor);
  }

  // Back button — đã cập nhật index thành 11 cho 7 phím
  SDL_FRect btnBack = {(WINDOW_WIDTH - 200) / 2.0f, 660, 200, 45};
  drawButton("< BACK", btnBack, hoveredItem == 11);
}

// ── drawTutorialScreen ───────────────────────────────────────
void Renderer::drawTutorialScreen(const char *keyNames[7]) {
  renderTextCentered(fontLarge, "HOW TO PLAY", WINDOW_WIDTH / 2, 60,
                     {233, 69, 96, 255});
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderLine(sdlRenderer, (float)(WINDOW_WIDTH / 2 - 150), 95,
                 (float)(WINDOW_WIDTH / 2 + 150), 95);

  SDL_Color white = {255, 255, 255, 255};
  SDL_Color gray = {150, 150, 170, 255};
  SDL_Color accent = {233, 69, 96, 255};
  SDL_Color green = {46, 204, 113, 255};

  // Controls panel
  drawPanel(80, 120, 400, 365, {20, 20, 45, 255}, {50, 50, 90, 255});
  renderText(fontMedium, "CONTROLS", 110, 135, accent);

  const char *labels[] = {"Move Left", "Move Right", "Soft Drop",
                          "Hard Drop", "Rotate CW", "Rotate CCW", "Hold"};
  for (int i = 0; i < 7; i++) {
    int yy = 175 + i * 40;
    renderText(fontSmall, labels[i], 110, yy, gray);
    
    // Key box co giãn động dựa trên độ dài chữ
    int textW = getTextWidth(fontSmall, keyNames[i]);
    float boxW = std::max(120.0f, (float)(textW + 24)); // padding 12px mỗi bên, tối thiểu 120px
    SDL_FRect kb = {460.0f - boxW, (float)(yy - 2), boxW, 26.0f};
    
    drawPanel(kb.x, kb.y, kb.w, kb.h, {30, 30, 60, 255}, {80, 80, 120, 255});
    renderTextCentered(fontSmall, keyNames[i], (int)(kb.x + kb.w / 2), yy + 10, green);
  }
  // ESC / Pause
  renderText(fontSmall, "Pause", 110, 455, gray);
  
  int textW = getTextWidth(fontSmall, "ESC");
  float boxW = std::max(120.0f, (float)(textW + 24));
  SDL_FRect kb = {460.0f - boxW, 453.0f, boxW, 26.0f};
  
  drawPanel(kb.x, kb.y, kb.w, kb.h, {30, 30, 60, 255}, {80, 80, 120, 255});
  renderTextCentered(fontSmall, "ESC", (int)(kb.x + kb.w / 2), 465, green);

  // Scoring panel
  drawPanel(500, 120, 390, 365, {20, 20, 45, 255}, {50, 50, 90, 255});
  renderText(fontMedium, "SCORING", 530, 135, accent);

  const char *scoreInfo[] = {"1 Line = 100 x Lv",  "2 Lines = 300 x Lv",
                             "3 Lines = 500 x Lv", "4 Lines = 800 x Lv",
                             "",
                             "Soft Drop: +1/cell", "Hard Drop: +2/cell",
                             "",
                             "Level up every",     "10 lines cleared"};
  for (int i = 0; i < 10; i++) {
    renderText(fontSmall, scoreInfo[i], 530, 175 + i * 21,
               i < 4 ? white : gray);
  }

  // Decorative blocks at bottom
  float bx = 200;
  float by = 500;
  TetrominoType types[] = {TetrominoType::I, TetrominoType::O, TetrominoType::T,
                           TetrominoType::S, TetrominoType::Z, TetrominoType::J,
                           TetrominoType::L};
  for (int i = 0; i < 7; i++) {
    drawBlock3D(bx + i * 80, by, 50, getTetrominoColor(types[i]));
  }

  // Tips
  renderTextCentered(fontSmall, "Complete lines to score points!",
                     WINDOW_WIDTH / 2, 580, gray);
  renderTextCentered(fontSmall, "Use HOLD to save a piece for later",
                     WINDOW_WIDTH / 2, 610, gray);

  // Back button
  SDL_FRect btnBack = {(WINDOW_WIDTH - 200) / 2.0f, 650, 200, 45};
  drawButton("< BACK", btnBack, false);
}

// ── drawPauseOverlay ─────────────────────────────────────────
void Renderer::drawPauseOverlay() {
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 160);
  SDL_FRect full = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
  SDL_RenderFillRect(sdlRenderer, &full);
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);

  // Pause panel
  float pw = 350, ph = 180;
  float px = (WINDOW_WIDTH - pw) / 2;
  float py = (WINDOW_HEIGHT - ph) / 2;
  drawPanel(px, py, pw, ph, {20, 20, 45, 240}, {233, 69, 96, 255});

  renderTextCentered(fontLarge, "PAUSED", WINDOW_WIDTH / 2, (int)(py + 60),
                     {233, 69, 96, 255});
  renderTextCentered(fontSmall, "Press ESC to continue", WINDOW_WIDTH / 2,
                     (int)(py + 120), {150, 150, 170, 255});
}

// ── drawGameOverScreen ───────────────────────────────────────
void Renderer::drawGameOverScreen(int currentScore, int currentLevel,
                                  int currentLines) {
  // Dark overlay
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 180);
  SDL_FRect full = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
  SDL_RenderFillRect(sdlRenderer, &full);
  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);

  // Popup panel
  float pw = 420, ph = 500;
  float px = (WINDOW_WIDTH - pw) / 2;
  float py = (WINDOW_HEIGHT - ph) / 2;
  drawPanel(px, py, pw, ph, {20, 20, 45, 255}, {231, 76, 60, 255});

  SDL_Color white = {255, 255, 255, 255};
  SDL_Color gray = {150, 150, 170, 255};
  SDL_Color gold = {255, 215, 0, 255};
  SDL_Color red = {231, 76, 60, 255};
  int cx = WINDOW_WIDTH / 2;

  renderTextCentered(fontLarge, "GAME OVER", cx, (int)(py + 40), red);

  // Current game stats
  char buf[64];
  renderTextCentered(fontSmall, "SCORE", cx - 100, (int)(py + 90), gray);
  snprintf(buf, sizeof(buf), "%d", currentScore);
  renderTextCentered(fontMedium, buf, cx - 100, (int)(py + 115), white);

  renderTextCentered(fontSmall, "LEVEL", cx, (int)(py + 90), gray);
  snprintf(buf, sizeof(buf), "%d", currentLevel);
  renderTextCentered(fontMedium, buf, cx, (int)(py + 115), white);

  renderTextCentered(fontSmall, "LINES", cx + 100, (int)(py + 90), gray);
  snprintf(buf, sizeof(buf), "%d", currentLines);
  renderTextCentered(fontMedium, buf, cx + 100, (int)(py + 115), white);

  // Separator
  SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 120, 255);
  SDL_RenderLine(sdlRenderer, px + 30, py + 145, px + pw - 30, py + 145);

  // Highscores
  if (!highscoresLoaded) {
    loadHighscores();
    highscores.push_back(currentScore);
    std::sort(highscores.rbegin(), highscores.rend());
    if (highscores.size() > 5)
      highscores.resize(5);
    std::ofstream file("highscores.txt");
    for (int s : highscores)
      file << s << "\n";
    highscoresLoaded = true;
  }

  renderTextCentered(fontMedium, "HIGH SCORES", cx, (int)(py + 170), gold);

  if (highscores.empty()) {
    renderTextCentered(fontSmall, "No scores yet!", cx, (int)(py + 210), gray);
  } else {
    bool highlighted = false;
    for (size_t i = 0; i < highscores.size(); i++) {
      SDL_Color sc = white;
      if (!highlighted && highscores[i] == currentScore) {
        sc = gold;
        highlighted = true;
      }
      snprintf(buf, sizeof(buf), "%d. %d", (int)(i + 1), highscores[i]);
      renderTextCentered(fontSmall, buf, cx, (int)(py + 210 + i * 32), sc);
    }
  }

  // Buttons
  float btnW = 160, btnH = 45;
  float btnY2 = py + ph - 65;
  SDL_FRect btnRestart = {(float)(cx - btnW - 15), btnY2, btnW, btnH};
  SDL_FRect btnHome = {(float)(cx + 15), btnY2, btnW, btnH};
  drawButton("RESTART", btnRestart, false, {46, 204, 113, 255});
  drawButton("MENU", btnHome, false, {52, 152, 219, 255});

  renderTextCentered(fontSmall, "ENTER = Menu", cx, (int)(btnY2 + 60),
                     {80, 80, 100, 255});
}
