#include "Tetromino.h"
#include "Board.h"

#include <algorithm>
#include <array>
#include <random>
#include <vector>

// Quan ly hinh dang, di chuyen, xoay va sinh manh ngau nhien.
namespace {
    // Ma tran bool 4x4 cho mot trang thai xoay.
    using Matrix4 = std::array<std::array<bool, TETROMINO_SIZE>, TETROMINO_SIZE>;

    // Tao ma tran 4x4 rong.
    Matrix4 emptyMatrix() {
        Matrix4 matrix{};
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                matrix[row][col] = false;
            }
        }
        return matrix;
    }

    // Trang thai khoi tao cho tung loai tetromino.
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

    // Xoay ma tran 4x4 theo chieu kim dong ho.
    Matrix4 rotateCWMatrix(const Matrix4& source) {
        Matrix4 rotated = emptyMatrix();

        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                rotated[col][TETROMINO_SIZE - 1 - row] = source[row][col];
            }
        }

        return rotated;
    }

    // Kiem tra va cham va bien cho vi tri thu.
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

                // Cho phep boardRow am khi manh moi xuat hien mot phan o tren bang.
                if (boardRow >= 0 && !board.isCellEmpty(boardCol, boardRow)) {
                    return false;
                }
            }
        }

        return true;
    }
}

// Khoi tao manh gan giua bang voi trang thai xoay ban dau.
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

    // Tinh san 4 trang thai xoay de truy van nhanh.
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

void Tetromino::moveUp() {
    --y;
}

void Tetromino::rotateCW() {
    rotation = (rotation + 1) % NUM_ROTATIONS;
}

void Tetromino::rotateCCW() {
    rotation = (rotation + NUM_ROTATIONS - 1) % NUM_ROTATIONS;
}

// Tinh vi tri Y ha canh de ve bong mo.
int Tetromino::getGhostY(const Board& board) const {
    int ghostY = y;

    while (canPieceExistAt(*this, board, x, ghostY + 1)) {
        ++ghostY;
    }

    return ghostY;
}

// Bo 7-bag: moi chu ky 7 manh se co du tat ca cac loai.
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

int Tetromino::getColorID() const {
    return (int)type;
}

int Tetromino::getBlockX(int index) const {
    int count = 0;
    for (int row = 0; row < TETROMINO_SIZE; ++row)
        for (int col = 0; col < TETROMINO_SIZE; ++col)
            if (shape[rotation][row][col]) {
                if (count == index) return col;
                ++count;
            }
    return 0;
}

int Tetromino::getBlockY(int index) const {
    int count = 0;
    for (int row = 0; row < TETROMINO_SIZE; ++row)
        for (int col = 0; col < TETROMINO_SIZE; ++col)
            if (shape[rotation][row][col]) {
                if (count == index) return row;
                ++count;
            }
    return 0;
}

