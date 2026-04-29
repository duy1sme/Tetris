#ifndef TETROMINO_H
#define TETROMINO_H

#include "GameState.h"

constexpr int TETROMINO_SIZE = 4; // Ma tran 4x4 cho moi trang thai.
constexpr int NUM_ROTATIONS = 4;  // 0/90/180/270 do.

class Tetromino {
private:
    TetrominoType type;
    int rotation; // [0..3]

    // Ma tran hinh dang theo [xoay][hang][cot].
    bool shape[NUM_ROTATIONS][TETROMINO_SIZE][TETROMINO_SIZE];

public:
    Tetromino(TetrominoType type);

    // Vi tri hien tai tren bang.
    int x, y;

    // Thong tin manh.
    TetrominoType getType() const;
    int getRotation() const;
    bool isCellFilled(int col, int row) const;

    // Di chuyen va xoay.
    void moveLeft();
    void moveRight();
    void moveDown();
    void moveUp(); // Dung de hoan tac buoc roi khong hop le.
    void rotateCW();
    void rotateCCW();

    // Hang du kien khi manh roi cham day (de ve bong mo).
    int getGhostY(const class Board& board) const;

    // Bo sinh ngau nhien 7-bag.
    static Tetromino createRandom();

    int getColorID() const;
    int getBlockX(int index) const;
    int getBlockY(int index) const;
};

#endif