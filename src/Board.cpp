#include "Board.h"

Board::Board() {
    reset();
}

void Board::reset() {
    for (int row = 0; row < BOARD_HEIGHT; row++)
        for (int col = 0; col < BOARD_WIDTH; col++)
            grid[row][col] = TetrominoType::NONE;
}

bool Board::isCellEmpty(int col, int row) const       { return true; }
TetrominoType Board::getCellType(int col, int row) const { return TetrominoType::NONE; }
bool Board::isInBounds(int col, int row) const        { return true; }
void Board::lockPiece(const Tetromino& tetromino)     { }
int  Board::clearLines()                              { return 0; }
bool Board::isGameOver() const                        { return false; }