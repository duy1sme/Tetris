// Include Guard    
#ifndef GAMESTATE_H
#define GAMESTATE_H

// HẰNG SỐ KÍCH THƯỚC
// Kích thước board
constexpr int BOARD_WIDTH  = 10;
constexpr int BOARD_HEIGHT = 20;

// Kích thước mỗi ô tính bằng pixel
constexpr int CELL_SIZE = 32;

// Vị trí board trên màn hình (offset từ góc trái trên)
constexpr int BOARD_OFFSET_X = 200;
constexpr int BOARD_OFFSET_Y = 40;

// Kích thước cửa sổ
constexpr int WINDOW_WIDTH  = 640;
constexpr int WINDOW_HEIGHT = 720;
/*constexpr thay vì #define — an toàn hơn, có kiểu dữ liệu rõ ràng, compiler kiểm tra được.*/

// ENUM GAMESTATE
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};
/*Dùng enum class thay vì enum — tránh xung đột tên, phải viết rõ GameState::PLAYING thay vì chỉ PLAYING.*/

// ENUM TETROMINOTYPE
enum class TetrominoType {
    I, O, T, S, Z, J, L,
    NONE   // ← dùng để biểu diễn ô trống trên board
};

// ENUM SOUNDTYPE
enum class SoundType {
    MOVE,
    ROTATE,
    LOCK,
    CLEAR_SINGLE,
    CLEAR_DOUBLE,
    CLEAR_TRIPLE,
    CLEAR_TETRIS,
    GAME_OVER
};

#endif