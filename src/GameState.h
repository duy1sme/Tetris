/*
 * GameState.h
 * -----------
 * File trung tâm chứa toàn bộ hằng số, enum và kiểu dữ liệu dùng chung
 * trong toàn bộ project. Mọi file khác đều include file này.
 *
 * Nội dung:
 *  - Hằng số kích thước cửa sổ, bảng chơi và ô vuông.
 *  - GameState  : trạng thái hiện tại của vòng lặp game.
 *  - TetrominoType : loại mảnh Tetris (I, O, T, S, Z, J, L, NONE).
 *  - SoundType  : sự kiện âm thanh dùng với AudioManager.
 *  - LevelConfig: cấu hình cho mỗi level (tốc độ, màu, tên).
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdint>

// ============================================================
// Cấu hình kích thước bảng chơi (tính theo đơn vị ô vuông).
// ============================================================
constexpr int BOARD_WIDTH  = 10; // Số cột
constexpr int BOARD_HEIGHT = 20; // Số hàng

// Kích thước pixel của mỗi ô vuông trên màn hình.
constexpr int CELL_SIZE = 32;

// Tọa độ góc trái-trên của bảng trong cửa sổ (pixel) — căn giữa.
constexpr int BOARD_OFFSET_X = 324;
constexpr int BOARD_OFFSET_Y = 44;

// Kích thước cửa sổ game (pixel).
constexpr int WINDOW_WIDTH  = 967;
constexpr int WINDOW_HEIGHT = 727;

// ============================================================
// Trạng thái chính của vòng lặp game.
// ============================================================
enum class GameState {
    MENU,         // Màn hình chính
    LEVEL_SELECT, // Chọn level trước khi chơi
    TUTORIAL,     // Màn hình hướng dẫn thao tác
    SETTINGS,     // Màn hình cài đặt
    PLAYING,      // Đang chơi
    PAUSED,       // Tạm dừng (ESC)
    GAME_OVER     // Kết thúc ván
};

// ============================================================
// Loại mảnh Tetris — cũng dùng làm giá trị ô trong bảng.
// ============================================================
enum class TetrominoType {
    I,    // Thanh dài 4 ô  — màu xanh lơ (cyan)
    O,    // Hình vuông 2x2  — màu vàng
    T,    // Hình chữ T      — màu tím
    S,    // Hình chữ S      — màu xanh lá
    Z,    // Hình chữ Z      — màu đỏ
    J,    // Hình chữ J      — màu xanh dương
    L,    // Hình chữ L      — màu cam
    NONE  // Ô trống (không có mảnh)
};

// ============================================================
// Sự kiện âm thanh dùng bởi AudioManager::playSFX().
// ============================================================
enum class SoundType {
    MOVE,          // Di chuyển trái/phải
    ROTATE,        // Xoay mảnh
    LOCK,          // Mảnh chạm đất và khóa lại
    CLEAR_SINGLE,  // Xóa 1 dòng
    CLEAR_DOUBLE,  // Xóa 2 dòng
    CLEAR_TRIPLE,  // Xóa 3 dòng
    CLEAR_TETRIS,  // Xóa 4 dòng (Tetris!)
    BUTTON,        // Click nút trên giao diện
    GAME_OVER      // Kết thúc ván chơi
};

// ============================================================
// Cấu hình cho mỗi level — tốc độ rơi khởi đầu và màu theme.
// ============================================================
struct LevelConfig {
    int   startLevel;         // Level khởi đầu
    float startFallInterval;  // Giây giữa mỗi lần rơi
    const char* name;         // Tên hiển thị
    uint8_t themeR, themeG, themeB; // Màu theme
};

constexpr int NUM_LEVELS = 3;

inline const LevelConfig LEVEL_CONFIGS[NUM_LEVELS] = {
    {1, 1.0f,  "EASY",    46, 204, 113},  // Xanh lá
    {3, 0.6f,  "NORMAL",  52, 152, 219},  // Xanh dương
    {5, 0.35f, "HARD",   231,  76,  60},  // Đỏ
};

#endif