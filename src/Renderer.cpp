#include "Renderer.h"

Renderer::Renderer(SDL_Renderer* renderer)
    : sdlRenderer(renderer) {
}

Renderer::~Renderer() { }

void Renderer::clear()   { }
void Renderer::present() { }
void Renderer::drawBoard(const Board& board)                                          { }
void Renderer::drawTetromino(const Tetromino& tetromino)                              { }
void Renderer::drawGhostPiece(const Tetromino& tetromino, int ghostY)                { }
void Renderer::drawNextPiece(const Tetromino& nextPiece)                             { }
void Renderer::drawUI(int score, int level, int lines)                               { }
void Renderer::drawScreen(GameState state)                                           { }
void Renderer::drawLineClearEffect(const Board& board, int clearedRows[], int count) { }