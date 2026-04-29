/*
 * Tetromino.h
 * -----------
 * Định nghĩa lớp Tetromino — đại diện cho một mảnh Tetris đang di chuyển.
 *
 * Mỗi mảnh được biểu diễn bằng:
 *  - type     : loại mảnh (I, O, T, S, Z, J, L).
 *  - rotation : góc xoay hiện tại [0..3] tương ứng 0°/90°/180°/270°.
 *  - shape    : mảng bool [NUM_ROTATIONS][TETROMINO_SIZE][TETROMINO_SIZE]
 *               được tính sẵn trong constructor để truy vấn O(1).
 *  - x, y     : vị trí góc trên-trái của ma trận 4×4 trên bảng chơi.
 *
 * Thuật toán xoay:
 *   Constructor tính trước 4 trạng thái xoay bằng cách nhân ma trận
 *   liên tiếp với rotateCWMatrix(). Khi xoay, chỉ cần thay đổi `rotation`.
 *
 * Bộ sinh ngẫu nhiên 7-bag:
 *   createRandom() đảm bảo mỗi chu kỳ 7 mảnh sẽ có đủ tất cả 7 loại,
 *   giống chuẩn Tetris Guideline.
 */

#ifndef TETROMINO_H
#define TETROMINO_H

#include "GameState.h"

constexpr int TETROMINO_SIZE = 4; // Ma trận 4×4 chứa hình dạng mảnh.
constexpr int NUM_ROTATIONS  = 4; // 4 góc xoay: 0°, 90°, 180°, 270°.

class Tetromino {
private:
    TetrominoType type;     // Loại mảnh (xác định hình dạng và màu sắc).
    int rotation;           // Chỉ số góc xoay hiện tại [0..3].

    // Ma trận hình dạng tính sẵn cho cả 4 góc xoay.
    // shape[rot][row][col] = true nếu ô (col, row) ở góc xoay rot có khối.
    bool shape[NUM_ROTATIONS][TETROMINO_SIZE][TETROMINO_SIZE];

public:
    // Khởi tạo mảnh với loại cho trước, đặt tại vị trí xuất hiện mặc định.
    Tetromino(TetrominoType type);

    // Vị trí góc trên-trái của ma trận 4×4 trên bảng chơi.
    // (x, y) = (3, 0) là vị trí xuất hiện mặc định (gần giữa bảng).
    int x, y;

    // ── Truy vấn ──────────────────────────────────────────────────────
    TetrominoType getType() const;      // Trả về loại mảnh.
    int getRotation() const;            // Trả về góc xoay hiện tại [0..3].

    // Trả về true nếu ô (col, row) ở góc xoay hiện tại có khối.
    bool isCellFilled(int col, int row) const;

    // ── Di chuyển ─────────────────────────────────────────────────────
    void moveLeft();   // x -= 1
    void moveRight();  // x += 1
    void moveDown();   // y += 1
    void moveUp();     // y -= 1 (dùng để hoàn tác bước rơi không hợp lệ)

    // ── Xoay ──────────────────────────────────────────────────────────
    void rotateCW();   // Xoay thuận chiều kim đồng hồ  (rotation + 1) % 4
    void rotateCCW();  // Xoay ngược chiều kim đồng hồ  (rotation + 3) % 4

    // ── Bóng mờ (Ghost Piece) ─────────────────────────────────────────
    // Tính hàng Y mà mảnh sẽ chạm đất nếu thả thẳng xuống.
    // Renderer dùng giá trị này để vẽ hình bóng mờ.
    int getGhostY(const class Board& board) const;

    // ── Bộ sinh ngẫu nhiên 7-bag ──────────────────────────────────────
    // Trả về mảnh ngẫu nhiên theo thuật toán 7-bag (mỗi 7 mảnh đủ loại).
    static Tetromino createRandom();

    // ── Tiện ích cho Renderer ─────────────────────────────────────────
    int getColorID() const;          // ID màu = cast TetrominoType sang int.
    int getBlockX(int index) const;  // Cột của khối thứ `index` trong mảnh.
    int getBlockY(int index) const;  // Hàng của khối thứ `index` trong mảnh.
};

#endif