#ifndef RENDERER_H
#define RENDERER_H

#include "GameState.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

// Khai bao truoc de tranh include vong.
class Board;
class Tetromino;

class Renderer {
private:
  SDL_Renderer *sdlRenderer;
  SDL_Texture *mainMenuTexture;
  SDL_Texture *gameScreenTexture;
  SDL_Texture *highScoreTexture;
  SDL_Texture *blockTexture;
  TTF_Font *font;

  std::vector<int> highscores;
  bool highscoresLoaded;
  void loadHighscores();

  // Hàm tiện ích nội bộ
  void renderText(const char *text, int x, int y, SDL_Color color);
  void renderTextCentered(const char *text, int cx, int cy, SDL_Color color);
  SDL_Color getTetrominoColor(TetrominoType type);

public:
  Renderer(SDL_Renderer *renderer);
  ~Renderer();

  // Hàm vẽ từng thành phần
  // Xóa màn hình, chuẩn bị frame mới
  void clear();

  // Hiển thị frame lên màn hình
  void present();

  // Ve bang choi (cac o da khoa)
  void drawBoard(const Board &board);

  // Vẽ mảnh đang rơi
  void drawTetromino(const Tetromino &tetromino);

  // Ve bong mo ha canh
  void drawGhostPiece(const Tetromino &tetromino, int ghostY);

  // Vẽ mảnh tiếp theo ở panel bên phải
  void drawNextPiece(const Tetromino &nextPiece);

  // Ve thong tin giao dien
  // Hien thi: diem, cap do, so dong
  void drawUI(int score, int level, int lines);

  // Ve man hinh theo trang thai hien tai
  void drawScreen(GameState state, int currentScore = 0, int currentLevel = 0, int currentLines = 0);
  /*Nhận vào GameState — tự biết vẽ màn hình nào
  MENU → vẽ main menu
  PAUSED → vẽ overlay pause
  GAME_OVER → vẽ màn hình game over*/

  // Hàm animation
  // Flash các dòng trước khi xóa
  void drawLineClearEffect(const Board &board, int clearedRows[], int count);
};

#endif