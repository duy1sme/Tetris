#include "Tetromino.h"
#include "Board.h"

Tetromino::Tetromino(TetrominoType type)
    : x(3)
    , y(0)
    , type(type)
    , rotation(0) {
}

TetrominoType Tetromino::getType()     const { return type; }
int           Tetromino::getRotation() const { return rotation; }
bool          Tetromino::isCellFilled(int col, int row) const { return false; }

void Tetromino::moveLeft()  { x--; }
void Tetromino::moveRight() { x++; }
void Tetromino::moveDown()  { y++; }

void Tetromino::rotateCW()  { rotation = (rotation + 1) % 4; }
void Tetromino::rotateCCW() { rotation = (rotation + 3) % 4; }

int Tetromino::getGhostY(const Board& board) const { return y; }

Tetromino Tetromino::createRandom() {
    return Tetromino(TetrominoType::T);  // tạm trả về T, TV1 sẽ làm ngẫu nhiên sau
}