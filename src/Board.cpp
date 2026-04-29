/*
 * Board.cpp
 * ---------
 * Triển khai Board — quản lý lưới ô đã khóa và logic xóa hàng.
 *
 * Sơ đồ tọa độ bảng:
 *   col  0 1 2 ... 9          (trục X, trái→phải)
 *   row  0                    (hàng trên cùng — game over nếu bị chiếm)
 *        1
 *        ...
 *       19                    (hàng dưới cùng)
 */

#include "Board.h"
#include "Tetromino.h"

// Constructor: khởi tạo bảng trống ngay khi tạo object.
Board::Board() {
    reset();
}

// Đặt toàn bộ ô về NONE (dùng đầu ván mới hoặc khi restart).
void Board::reset() {
    for (int row = 0; row < BOARD_HEIGHT; ++row) {
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[row][col] = TetrominoType::NONE;
        }
    }
}

// Trả về true nếu (col, row) nằm trong giới hạn bảng.
bool Board::isInBounds(int col, int row) const {
    return col >= 0 && col < BOARD_WIDTH && row >= 0 && row < BOARD_HEIGHT;
}

// Ô ngoài biên được coi là đã chiếm để đảm bảo va chạm biên đúng.
bool Board::isCellEmpty(int col, int row) const {
    if (!isInBounds(col, row)) {
        return false; // Ngoài biên → không trống
    }
    return grid[row][col] == TetrominoType::NONE;
}

// Trả về NONE nếu tọa độ không hợp lệ để tránh truy cập mảng sai.
TetrominoType Board::getCellType(int col, int row) const {
    if (!isInBounds(col, row)) {
        return TetrominoType::NONE;
    }
    return grid[row][col];
}

// Ghi các ô của mảnh vào lưới khi mảnh chạm đất.
// Chỉ ghi những ô nằm trong bảng (ô trên cùng đang xuất hiện có thể nằm
// ngoài biên trên — boardRow < 0 — nên cần kiểm tra isInBounds).
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

// Quét từ dưới lên để xóa hàng đầy, kéo các hàng phía trên xuống.
// Quét từ dưới lên cho phép xử lý nhiều hàng đầy liên tiếp chính xác:
// sau khi xóa và kéo xuống, index `row` được tăng lên 1 (++row ở cuối
// vòng lặp for sẽ cân bằng với --row ở bước kéo, nên cùng hàng đó được
// kiểm tra lại).
int Board::clearLines() {
    int clearedLines = 0;

    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool fullLine = true;

        // Kiểm tra hàng row có đầy không.
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (grid[row][col] == TetrominoType::NONE) {
                fullLine = false;
                break;
            }
        }

        if (!fullLine) {
            continue; // Hàng chưa đầy → bỏ qua
        }

        ++clearedLines;

        // Kéo tất cả hàng phía trên xuống 1 hàng (ghi đè hàng vừa xóa).
        for (int moveRow = row; moveRow > 0; --moveRow) {
            for (int col = 0; col < BOARD_WIDTH; ++col) {
                grid[moveRow][col] = grid[moveRow - 1][col];
            }
        }

        // Xóa hàng trên cùng (hàng 0) sau khi kéo xuống.
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[0][col] = TetrominoType::NONE;
        }

        // Kiểm tra lại chính hàng vừa kéo xuống (có thể cũng đầy).
        ++row;
    }

    return clearedLines;
}

// Game over khi hàng trên cùng (row 0) có ít nhất một ô bị chiếm.
bool Board::isGameOver() const {
    for (int col = 0; col < BOARD_WIDTH; ++col) {
        if (grid[0][col] != TetrominoType::NONE) {
            return true;
        }
    }
    return false;
}

// Trả về ID số nguyên của loại mảnh — Renderer dùng để chọn màu.
int Board::getCellColorID(int col, int row) const {
    return (int)getCellType(col, row);
}

// Kiểm tra mảnh có thể đặt tại vị trí tùy chỉnh (toX, toY) không.
// Dùng trong getGhostY() và các bước kiểm tra trước khi di chuyển.
bool Board::isValidPosition(const Tetromino& piece, int toX, int toY) const {
    for (int row = 0; row < TETROMINO_SIZE; ++row) {
        for (int col = 0; col < TETROMINO_SIZE; ++col) {
            if (!piece.isCellFilled(col, row)) continue;

            int boardCol = toX + col;
            int boardRow = toY + row;

            // Vượt biên trái, phải hoặc dưới → không hợp lệ.
            if (boardCol < 0 || boardCol >= BOARD_WIDTH || boardRow >= BOARD_HEIGHT)
                return false;

            // Cho phép boardRow âm (mảnh xuất hiện một phần ở trên bảng).
            if (boardRow >= 0 && !isCellEmpty(boardCol, boardRow))
                return false;
        }
    }
    return true;
}