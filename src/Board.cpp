#include "Board.h"
#include "Tetromino.h"

// ==========================
// File Board.cpp
// Nhiệm vụ: Quản lý bàn chơi Tetris.
// Board lưu trạng thái các ô đã cố định sau khi mảnh rơi xuống.
// Mỗi ô trong grid có thể là NONE nếu trống, hoặc là loại mảnh I/O/T/S/Z/J/L.
// ==========================

// Constructor: khi tạo Board mới thì gọi reset() để bàn chơi bắt đầu ở trạng thái trống.
Board::Board() {
    reset();
}

// reset(): Xóa toàn bộ bàn chơi.
// Dùng khi bắt đầu game mới hoặc chơi lại từ đầu.
// Tất cả ô trong grid được đưa về TetrominoType::NONE, nghĩa là ô trống.
void Board::reset() {
    for (int row = 0; row < BOARD_HEIGHT; ++row) {
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[row][col] = TetrominoType::NONE;
        }
    }
}

// isInBounds(): Kiểm tra tọa độ (col, row) có nằm trong giới hạn bàn chơi không.
// col là cột, row là hàng.
// Hàm này giúp tránh truy cập ra ngoài mảng grid.
bool Board::isInBounds(int col, int row) const {
    return col >= 0 && col < BOARD_WIDTH && row >= 0 && row < BOARD_HEIGHT;
}

// isCellEmpty(): Kiểm tra một ô trên board có trống không.
// Nếu ô nằm ngoài board thì trả về false để không cho khối đi ra ngoài biên.
// Nếu ô trong board và giá trị là NONE thì ô đó trống.
bool Board::isCellEmpty(int col, int row) const {
    if (!isInBounds(col, row)) {
        return false;
    }

    return grid[row][col] == TetrominoType::NONE;
}

// getCellType(): Lấy loại mảnh đang nằm tại ô (col, row).
// Phần Renderer có thể dùng hàm này để biết ô đó cần vẽ màu gì.
// Nếu tọa độ không hợp lệ thì trả về NONE để tránh lỗi truy cập mảng.
TetrominoType Board::getCellType(int col, int row) const {
    if (!isInBounds(col, row)) {
        return TetrominoType::NONE;
    }

    return grid[row][col];
}

// lockPiece(): Cố định mảnh Tetromino hiện tại vào board.
// Hàm này được gọi khi mảnh không thể rơi xuống nữa.
// Cách làm:
// 1. Duyệt ma trận 4x4 của mảnh.
// 2. Ô nào của mảnh đang được lấp thì tính vị trí thật trên board.
// 3. Ghi loại mảnh đó vào grid để biến nó thành khối cố định.
void Board::lockPiece(const Tetromino& tetromino) {
    for (int row = 0; row < TETROMINO_SIZE; ++row) {
        for (int col = 0; col < TETROMINO_SIZE; ++col) {
            // Bỏ qua các ô trống trong ma trận 4x4 của mảnh.
            if (!tetromino.isCellFilled(col, row)) {
                continue;
            }

            // Chuyển tọa độ trong mảnh 4x4 sang tọa độ thật trên board.
            const int boardCol = tetromino.x + col;
            const int boardRow = tetromino.y + row;

            // Chỉ ghi vào grid nếu vị trí nằm trong bàn chơi.
            if (isInBounds(boardCol, boardRow)) {
                grid[boardRow][boardCol] = tetromino.getType();
            }
        }
    }
}

// clearLines(): Kiểm tra và xóa các hàng đã đầy.
// Một hàng được coi là đầy khi tất cả ô trong hàng đều khác NONE.
// Sau khi xóa, các hàng phía trên sẽ được kéo xuống một dòng.
// Hàm trả về số hàng đã xóa để Game.cpp có thể dùng tính điểm.
int Board::clearLines() {
    int clearedLines = 0;

    // Duyệt từ dưới lên vì trong Tetris hàng dưới thường được lấp trước.
    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool fullLine = true;

        // Kiểm tra xem hàng hiện tại có ô trống không.
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (grid[row][col] == TetrominoType::NONE) {
                fullLine = false;
                break;
            }
        }

        // Nếu hàng chưa đầy thì bỏ qua.
        if (!fullLine) {
            continue;
        }

        ++clearedLines;

        // Kéo toàn bộ các hàng phía trên xuống một dòng.
        for (int moveRow = row; moveRow > 0; --moveRow) {
            for (int col = 0; col < BOARD_WIDTH; ++col) {
                grid[moveRow][col] = grid[moveRow - 1][col];
            }
        }

        // Sau khi kéo xuống, hàng trên cùng phải được đặt thành trống.
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            grid[0][col] = TetrominoType::NONE;
        }

        // Vì vừa kéo hàng phía trên xuống vị trí row,
        // cần kiểm tra lại chính hàng này để không bỏ sót nếu có nhiều hàng đầy liên tiếp.
        ++row;
    }

    return clearedLines;
}

// isGameOver(): Kiểm tra điều kiện thua.
// Nếu hàng đầu tiên đã có khối cố định thì nghĩa là các khối đã chạm tới đỉnh bàn chơi.
// Khi đó game kết thúc.
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