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
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

// ============================================================
// Cấu hình kích thước bảng chơi (tính theo đơn vị ô vuông).
// ============================================================
constexpr int BOARD_WIDTH  = 10; // Số cột
constexpr int BOARD_HEIGHT = 20; // Số hàng

// Kích thước pixel của mỗi ô vuông trên màn hình.
constexpr int CELL_SIZE = 32;

// Tọa độ góc trái-trên của bảng trong cửa sổ (pixel).
constexpr int BOARD_OFFSET_X = 326;
constexpr int BOARD_OFFSET_Y = 41;

// Kích thước cửa sổ game (pixel).
constexpr int WINDOW_WIDTH  = 967;
constexpr int WINDOW_HEIGHT = 727;

// ============================================================
// Trạng thái chính của vòng lặp game.
// Game::changeState() điều khiển việc chuyển đổi giữa các trạng thái.
// ============================================================
enum class GameState {
    MENU,      // Màn hình chính (chờ người chơi bắt đầu)
    TUTORIAL, // Màn hình hướng dẫn thao tác
    PLAYING,   // Đang chơi (vòng lặp cập nhật đầy đủ)
    PAUSED,    // Tạm dừng (ESC), không cập nhật logic
    SETTINGS,  // Màn hình cài đặt (chưa triển khai đầy đủ)
    GAME_OVER  // Kết thúc ván, hiển thị bảng điểm
};

// ============================================================
// Loại dữ liệu lưu trong mỗi ô của bảng chơi.
// Giá trị NONE = ô trống; các giá trị còn lại tương ứng màu sắc
// của từng loại mảnh Tetris theo chuẩn Tetris Guideline.
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
// Mỗi giá trị tương ứng một loại âm thanh hiệu ứng (SFX).
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

#endif