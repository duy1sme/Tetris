// Include Guard + Include cần thiết
#ifndef BOARD_H
#define BOARD_H

#include "GameState.h"
// Khai báo class Board
class Board
{
    private:
        TetrominoType grid[BOARD_HEIGHT][BOARD_WIDTH];
    public:
        Board();
        void reset();
        /*grid là mảng 2D — mỗi ô lưu TetrominoType để biết ô đó trống (NONE) hay đang có mảnh màu gì
        reset() — xóa toàn bộ board về trạng thái trống, dùng khi bắt đầu ván mới
        Board() — constructor, gọi reset() ngay khi tạo*/
        // Các hàm truy vấn (query):

        // Kiểm tra ô (col, row) có trống không
        bool isCellEmpty(int col, int row) const;

        // Lấy loại mảnh tại ô (col, row) - TV2 dùng để biết vẽ màu gì
        TetrominoType getCellType(int col, int row) const;

        // Kiểm tra tọa độ có nằm trong giới hạn board không
        bool isInBounds(int col, int row) const;

        // Các hàm thay đổi (mutator):
        // Khóa mảnh vào board khi nó chạm đáy
        void lockPiece(const class Tetromino& tetromino);

        // Xóa các dòng đầy, trả về số dòng đã xóa
        int clearLines();

        // Kiểm tra game over (có ô nào bị lấp ở hàng trên cùng không)
        bool isGameOver() const;

        int  getCellColorID(int col, int row) const;
        bool isValidPosition(const class Tetromino& piece, int toX, int toY) const; 
};
#endif