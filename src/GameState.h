#ifndef GAMESTATE_H
#define GAMESTATE_H

// Cau hinh kich thuoc bang choi.
constexpr int BOARD_WIDTH  = 10;
constexpr int BOARD_HEIGHT = 20;

// Kich thuoc pixel cua moi o.
constexpr int CELL_SIZE = 32;

// Toa do goc trai tren cua bang trong cua so.
constexpr int BOARD_OFFSET_X = 326;
constexpr int BOARD_OFFSET_Y = 41;

// Kich thuoc cua so game.
constexpr int WINDOW_WIDTH  = 967;
constexpr int WINDOW_HEIGHT = 727;

// Cac trang thai chinh cua vong lap game.
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS,
    GAME_OVER
};
// Loai du lieu luu trong tung o cua bang.
enum class TetrominoType {
    I, O, T, S, Z, J, L,
    NONE
};

// Cac su kien am thanh su dung boi AudioManager.
enum class SoundType {
    MOVE,
    ROTATE,
    LOCK,
    CLEAR_SINGLE,
    CLEAR_DOUBLE,
    CLEAR_TRIPLE,
    CLEAR_TETRIS,
    BUTTON,
    GAME_OVER
};

#endif