#include "Board.h"
#include "Tetromino.h"

// Quan ly cac o da khoa va logic xoa dong.
Board::Board() {
    reset();
}

// Dat toan bo bang ve trang thai trong.
void Board::reset() {
    for (int row = 0; row < BOARD_HEIGHT; ++row) {
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[row][col] = TetrominoType::NONE;
        }
    }
}

// Kiem tra toa do co nam trong gioi han bang khong.
bool Board::isInBounds(int col, int row) const {
    return col >= 0 && col < BOARD_WIDTH && row >= 0 && row < BOARD_HEIGHT;
}

// Toa do ngoai bien duoc xem la da chiem de dam bao va cham.
bool Board::isCellEmpty(int col, int row) const {
    if (!isInBounds(col, row)) {
        return false;
    }

    return grid[row][col] == TetrominoType::NONE;
}

// Tra ve NONE neu toa do khong hop le de tranh truy cap sai.
TetrominoType Board::getCellType(int col, int row) const {
    if (!isInBounds(col, row)) {
        return TetrominoType::NONE;
    }

    return grid[row][col];
}

// Ghi cac o cua manh hien tai vao grid.
void Board::lockPiece(const Tetromino& tetromino) {
    for (int row = 0; row < TETROMINO_SIZE; ++row) {
        for (int col = 0; col < TETROMINO_SIZE; ++col) {
            if (!tetromino.isCellFilled(col, row)) {
                continue;
            }

            const int boardCol = tetromino.x + col;
            const int boardRow = tetromino.y + row;

            if (isInBounds(boardCol, boardRow)) {
                grid[boardRow][boardCol] = tetromino.getType();
            }
        }
    }
}

// Xoa dong day va keo cac dong phia tren xuong.
int Board::clearLines() {
    int clearedLines = 0;

    // Duyet tu duoi len; sau khi xoa can kiem tra lai cung chi so dong.
    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool fullLine = true;

        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (grid[row][col] == TetrominoType::NONE) {
                fullLine = false;
                break;
            }
        }

        if (!fullLine) {
            continue;
        }

        ++clearedLines;

        for (int moveRow = row; moveRow > 0; --moveRow) {
            for (int col = 0; col < BOARD_WIDTH; ++col) {
                grid[moveRow][col] = grid[moveRow - 1][col];
            }
        }

        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[0][col] = TetrominoType::NONE;
        }

        // Kiem tra lai dong hien tai sau khi da keo xuong.
        ++row;
    }

    return clearedLines;
}

// Ket thuc van neu hang tren cung co it nhat mot o da bi chiem.
bool Board::isGameOver() const {
    for (int col = 0; col < BOARD_WIDTH; ++col) {
        if (grid[0][col] != TetrominoType::NONE) {
            return true;
        }
    }

    return false;
}

int Board::getCellColorID(int col, int row) const {
    return (int)getCellType(col, row);
}

bool Board::isValidPosition(const Tetromino& piece, int toX, int toY) const {
    for (int row = 0; row < TETROMINO_SIZE; ++row) {
        for (int col = 0; col < TETROMINO_SIZE; ++col) {
            if (!piece.isCellFilled(col, row)) continue;

            int boardCol = toX + col;
            int boardRow = toY + row;

            if (boardCol < 0 || boardCol >= BOARD_WIDTH || boardRow >= BOARD_HEIGHT)
                return false;

            if (boardRow >= 0 && !isCellEmpty(boardCol, boardRow))
                return false;
        }
    }
    return true;
}