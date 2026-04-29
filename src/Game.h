#ifndef GAME_H
#define GAME_H

#include "AudioManager.h"
#include "Board.h"
#include "GameState.h"
#include "Renderer.h"
#include "Tetromino.h"
#include <SDL3/SDL.h>


class Game {
private:
  // Cac doi tuong SDL luc chay.
  SDL_Window *window;
  SDL_Renderer *sdlRenderer;

  // Trang thai luc chay.
  GameState state;
  bool running;

  // Cac thanh phan chinh.
  Board board;
  Tetromino *currentPiece;
  Tetromino *nextPiece;
  Renderer *renderer;
  AudioManager audio;

  // Ba buoc xu ly moi frame.
  void handleInput();
  void update(float deltaTime);
  void render();

  // Cac ham tro giup gameplay.
  void spawnPiece();
  bool isValidPosition(const Tetromino &piece) const;
  void lockCurrentPiece();
  void calculateScore(int linesCleared);
  void changeState(GameState newState);

  // Du lieu diem va tien trinh.
  int score;
  int level;
  int totalLines;

  // Bo dem roi tu dong (don vi giay).
  float fallTimer;
  float fallInterval;

  // Vung nhan menu (du phong).
  SDL_FRect btnPlay     = {370, 305, 230, 55};
  SDL_FRect btnSettings = {350, 650, 60, 60};
  SDL_FRect btnHelp     = {435, 650, 60, 60};

  bool isMouseInRect(int mouseX, int mouseY, SDL_FRect rect);

  void hardDrop();
public:
  Game();
  ~Game();

  // Vong doi game.
  bool init();
  void run();
  void shutdown();
};

#endif