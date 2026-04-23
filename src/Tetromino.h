// File này mô tả 7 mảnh Tetris và cách chúng di chuyển.
#ifndef TETROMINO_H
#define TETROMINO_H

#include "GameState.h"

constexpr int TETROMINO_SIZE = 4; // ma trận 4x4
constexpr int NUM_ROTATIONS = 4; // mỗi mảnh có 4 trạng thái xoay 

// class Tetromino
class Tetromino
{
private:
    TetrominoType type;
    int rotation; // 0, 1, 2, 3

    bool shape[NUM_ROTATIONS][TETROMINO_SIZE][TETROMINO_SIZE];
    /*Chiều 1 — 4 trạng thái xoay (0°, 90°, 180°, 270°)
    Chiều 2, 3 — ma trận 4×4 mô tả hình dạng
    true = ô có khối, false = ô trống*/
public:
    Tetromino(TetrominoType type);

    int x,y; // Vị trí hiện tại trên board (cột, hàng)
    // Lấy loại mảnh - Board và Renderer cần để biết màu
    TetrominoType getType() const;

    // Lấy trạng thái xoay hiện tại
    int getRotation() const;

    // Kiểm tra ô (col, row) trong ma trận hiện tại có khối không
    bool isCellFilled(int col, int row) const;

    // HÀM DI CHUYỂN & XOAY
    // Di chuyển - chỉ thay đổi x, y
    void moveLeft();
    void moveRight();
    void moveDown();

    // Xoay - thay đổi rotation (0->1->2->3->0)
    void rotateCW(); // clockwise (theo chiều kim đồng hồ)
    void rotateCCW(); // counter-clockwise (ngược chiều kim đồng hồ)

    // Lấy vị trí ghost piece (bóng mờ phía dưới)
    int getGhostY(const class Board& board) const;

    /*rotateCW() và rotateCCW() — người chơi thường dùng 2 phím xoay khác nhau. 
    getGhostY() trả về tọa độ Y thấp nhất mảnh có thể rơi tới — TV2 dùng để vẽ bóng.*/

    // Tạo mảnh ngẫu nhiên theo bag randomizer
    static Tetromino createRandom();
    /*static nghĩa là gọi không cần object: Tetromino::createRandom(). 
    TV1 sẽ implement thuật toán bag randomizer — đảm bảo 7 mảnh xuất hiện đều nhau.*/
};

#endif