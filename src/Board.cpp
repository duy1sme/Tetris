#include "Board.h"
#include "Tetromino.h"

Board::Board() {
    reset();
}

void Board::reset() {
    for (int row = 0; row < BOARD_HEIGHT; ++row) {
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[row][col] = TetrominoType::NONE;
        }
    }
}

bool Board::isInBounds(int col, int row) const {
    return col >= 0 && col < BOARD_WIDTH && row >= 0 && row < BOARD_HEIGHT;
}

bool Board::isCellEmpty(int col, int row) const {
    if (!isInBounds(col, row)) {
        return false;
    }

    return grid[row][col] == TetrominoType::NONE;
}

TetrominoType Board::getCellType(int col, int row) const {
    if (!isInBounds(col, row)) {
        return TetrominoType::NONE;
    }

    return grid[row][col];
}

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

int Board::clearLines() {
    int clearedLines = 0;

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

        // Sau khi kéo dòng phía trên xuống, cần kiểm tra lại đúng dòng này.
        ++row;
    }

    return clearedLines;
}

bool Board::isGameOver() const {
    for (int col = 0; col < BOARD_WIDTH; ++col) {
        if (grid[0][col] != TetrominoType::NONE) {
            return true;
        }
    }

    return false;
}