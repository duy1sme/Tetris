/*
 * Board.h
 * -------
 * Quản lý trạng thái lưới (grid) của bảng chơi Tetris.
 *
 * Board lưu trữ tất cả các ô đã bị khóa (locked) trên bảng dưới dạng
 * mảng 2D TetrominoType[BOARD_HEIGHT][BOARD_WIDTH].
 * Ô trống được biểu diễn bằng TetrominoType::NONE.
 *
 * Các trách nhiệm chính:
 *  - Kiểm tra ô trống / biên bảng.
 *  - Ghi mảnh vào bảng khi nó chạm đất (lockPiece).
 *  - Phát hiện và xóa hàng đầy (clearLines).
 *  - Phát hiện trạng thái thua (isGameOver).
 */

#ifndef BOARD_H
#define BOARD_H

#include "GameState.h"

class Board {
private:
    // Lưới 2D lưu loại mảnh đã khóa tại từng ô.
    // Truy cập: grid[row][col], row 0 = hàng trên cùng.
    TetrominoType grid[BOARD_HEIGHT][BOARD_WIDTH];

public:
    Board();

    // Xóa toàn bộ bảng về trạng thái trống (dùng khi bắt đầu ván mới).
    void reset();

    // ── Nhóm truy vấn ──────────────────────────────────────────────────
    // Trả về true nếu ô tại (col, row) trống.
    // Tọa độ ngoài biên luôn trả về false (coi như đã chiếm).
    bool isCellEmpty(int col, int row) const;

    // Trả về loại mảnh đang chiếm ô (col, row).
    // Trả về NONE nếu ô trống hoặc ngoài biên.
    TetrominoType getCellType(int col, int row) const;

    // Trả về true nếu (col, row) nằm trong giới hạn bảng.
    bool isInBounds(int col, int row) const;

    // ── Nhóm cập nhật trạng thái ───────────────────────────────────────
    // Ghi tất cả ô của mảnh Tetromino vào lưới (gọi khi mảnh chạm đất).
    void lockPiece(const class Tetromino& tetromino);

    // Xóa tất cả hàng đầy, kéo các hàng phía trên xuống.
    // Trả về số hàng đã xóa (dùng để tính điểm).
    int clearLines();

    // Trả về true nếu hàng trên cùng (row 0) có ít nhất một ô bị chiếm
    // → điều kiện thua.
    bool isGameOver() const;

    // ── Tiện ích ───────────────────────────────────────────────────────
    // Trả về ID màu của ô (bằng cách cast TetrominoType sang int).
    int getCellColorID(int col, int row) const;

    // Kiểm tra mảnh có thể đặt tại vị trí (toX, toY) không.
    bool isValidPosition(const class Tetromino& piece, int toX, int toY) const;
};

#endif