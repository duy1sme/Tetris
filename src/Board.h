#ifndef BOARD_H
#define BOARD_H

#include "GameState.h"
class Board {
private:
    TetrominoType grid[BOARD_HEIGHT][BOARD_WIDTH];

public:
    Board();
    void reset();

    // Nhom ham truy van.
    bool isCellEmpty(int col, int row) const;
    TetrominoType getCellType(int col, int row) const;
    bool isInBounds(int col, int row) const;

    // Nhom ham cap nhat trang thai.
    void lockPiece(const class Tetromino& tetromino);
    int clearLines();
    bool isGameOver() const;

    int getCellColorID(int col, int row) const;
    bool isValidPosition(const class Tetromino& piece, int toX, int toY) const;
};

#endif