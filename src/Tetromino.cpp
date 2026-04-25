#include "Tetromino.h"
#include "Board.h"

#include <algorithm>
#include <array>
#include <random>
#include <vector>

namespace {
    using Matrix4 = std::array<std::array<bool, TETROMINO_SIZE>, TETROMINO_SIZE>;

    Matrix4 emptyMatrix() {
        Matrix4 matrix{};
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                matrix[row][col] = false;
            }
        }
        return matrix;
    }

    Matrix4 getBaseShape(TetrominoType type) {
        Matrix4 shape = emptyMatrix();

        switch (type) {
            case TetrominoType::I:
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                shape[1][3] = true;
                break;

            case TetrominoType::O:
                shape[0][1] = true;
                shape[0][2] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            case TetrominoType::T:
                shape[0][1] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            case TetrominoType::S:
                shape[0][1] = true;
                shape[0][2] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                break;

            case TetrominoType::Z:
                shape[0][0] = true;
                shape[0][1] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            case TetrominoType::J:
                shape[0][0] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            case TetrominoType::L:
                shape[0][2] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            case TetrominoType::NONE:
            default:
                break;
        }

        return shape;
    }

    Matrix4 rotateCWMatrix(const Matrix4& source) {
        Matrix4 rotated = emptyMatrix();

        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                rotated[col][TETROMINO_SIZE - 1 - row] = source[row][col];
            }
        }

        return rotated;
    }

    bool canPieceExistAt(const Tetromino& tetromino, const Board& board, int testX, int testY) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                if (!tetromino.isCellFilled(col, row)) {
                    continue;
                }

                const int boardCol = testX + col;
                const int boardRow = testY + row;

                if (boardCol < 0 || boardCol >= BOARD_WIDTH || boardRow >= BOARD_HEIGHT) {
                    return false;
                }

                if (boardRow >= 0 && !board.isCellEmpty(boardCol, boardRow)) {
                    return false;
                }
            }
        }

        return true;
    }
}

Tetromino::Tetromino(TetrominoType type)
    : type(type)
    , rotation(0)
    , x(3)
    , y(0) {

    for (int rot = 0; rot < NUM_ROTATIONS; ++rot) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                shape[rot][row][col] = false;
            }
        }
    }

    Matrix4 current = getBaseShape(type);

    for (int rot = 0; rot < NUM_ROTATIONS; ++rot) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                shape[rot][row][col] = current[row][col];
            }
        }

        current = rotateCWMatrix(current);
    }
}

TetrominoType Tetromino::getType() const {
    return type;
}

int Tetromino::getRotation() const {
    return rotation;
}

bool Tetromino::isCellFilled(int col, int row) const {
    if (col < 0 || col >= TETROMINO_SIZE || row < 0 || row >= TETROMINO_SIZE) {
        return false;
    }

    return shape[rotation][row][col];
}

void Tetromino::moveLeft() {
    --x;
}

void Tetromino::moveRight() {
    ++x;
}

void Tetromino::moveDown() {
    ++y;
}

void Tetromino::rotateCW() {
    rotation = (rotation + 1) % NUM_ROTATIONS;
}

void Tetromino::rotateCCW() {
    rotation = (rotation + NUM_ROTATIONS - 1) % NUM_ROTATIONS;
}

int Tetromino::getGhostY(const Board& board) const {
    int ghostY = y;

    while (canPieceExistAt(*this, board, x, ghostY + 1)) {
        ++ghostY;
    }

    return ghostY;
}

Tetromino Tetromino::createRandom() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::vector<TetrominoType> bag;

    if (bag.empty()) {
        bag = {
            TetrominoType::I,
            TetrominoType::O,
            TetrominoType::T,
            TetrominoType::S,
            TetrominoType::Z,
            TetrominoType::J,
            TetrominoType::L
        };

        std::shuffle(bag.begin(), bag.end(), generator);
    }

    const TetrominoType nextType = bag.back();
    bag.pop_back();

    return Tetromino(nextType);
}