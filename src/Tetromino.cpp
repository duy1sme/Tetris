/*
 * Tetromino.cpp
 * -------------
 * Triển khai lớp Tetromino — quản lý hình dạng, di chuyển, xoay
 * và bộ sinh mảnh ngẫu nhiên 7-bag.
 *
 * Các chi tiết kỹ thuật quan trọng:
 *
 * 1. Hình dạng được định nghĩa trong getBaseShape() bằng ma trận 4×4.
 *    Quy ước: shape[row][col], row 0 = hàng trên cùng.
 *
 * 2. Constructor tính sẵn 4 góc xoay trong mảng shape[4][4][4] để
 *    truy vấn isCellFilled() luôn là O(1), không xoay lúc runtime.
 *
 * 3. rotateCWMatrix() áp dụng công thức xoay ma trận 4×4:
 *      rotated[col][SIZE-1-row] = source[row][col]
 *
 * 4. 7-bag dùng std::shuffle với Mersenne Twister để đảm bảo phân phối
 *    đều, tránh chuỗi dài cùng một loại mảnh.
 */

#include "Tetromino.h"
#include "Board.h"

#include <algorithm>
#include <array>
#include <random>
#include <vector>

// ============================================================
// Hàm nội bộ (ẩn trong anonymous namespace, không dùng ở file khác)
// ============================================================
namespace {
    // Alias ngắn gọn cho ma trận bool 4×4.
    using Matrix4 = std::array<std::array<bool, TETROMINO_SIZE>, TETROMINO_SIZE>;

    // Tạo ma trận 4×4 toàn false.
    Matrix4 emptyMatrix() {
        Matrix4 matrix{};
        for (int row = 0; row < TETROMINO_SIZE; ++row)
            for (int col = 0; col < TETROMINO_SIZE; ++col)
                matrix[row][col] = false;
        return matrix;
    }

    // Trả về ma trận hình dạng cơ bản (trạng thái 0°) cho từng loại mảnh.
    // Vị trí các ô true tương ứng hình dạng mảnh theo chuẩn Tetris Guideline.
    Matrix4 getBaseShape(TetrominoType type) {
        Matrix4 shape = emptyMatrix();

        switch (type) {
            case TetrominoType::I:
                // Hàng 1: □□□□  (4 ô ngang liền nhau)
                shape[1][0] = shape[1][1] = shape[1][2] = shape[1][3] = true;
                break;

            case TetrominoType::O:
                // Hình vuông 2×2 tại góc (1,0)
                shape[0][1] = shape[0][2] = shape[1][1] = shape[1][2] = true;
                break;

            case TetrominoType::T:
                // Hình chữ T: đầu lên trên
                shape[0][1] = true;
                shape[1][0] = shape[1][1] = shape[1][2] = true;
                break;

            case TetrominoType::S:
                // S: hai ô phải ở hàng trên, hai ô trái ở hàng dưới
                shape[0][1] = shape[0][2] = true;
                shape[1][0] = shape[1][1] = true;
                break;

            case TetrominoType::Z:
                // Z: hai ô trái ở hàng trên, hai ô phải ở hàng dưới
                shape[0][0] = shape[0][1] = true;
                shape[1][1] = shape[1][2] = true;
                break;

            case TetrominoType::J:
                // J: móc hướng trái
                shape[0][0] = true;
                shape[1][0] = shape[1][1] = shape[1][2] = true;
                break;

            case TetrominoType::L:
                // L: móc hướng phải
                shape[0][2] = true;
                shape[1][0] = shape[1][1] = shape[1][2] = true;
                break;

            case TetrominoType::NONE:
            default:
                break;
        }

        return shape;
    }

    // Xoay ma trận 4×4 thuận chiều kim đồng hồ 90°.
    // Công thức: rotated[col][SIZE-1-row] = source[row][col]
    Matrix4 rotateCWMatrix(const Matrix4& source) {
        Matrix4 rotated = emptyMatrix();
        for (int row = 0; row < TETROMINO_SIZE; ++row)
            for (int col = 0; col < TETROMINO_SIZE; ++col)
                rotated[col][TETROMINO_SIZE - 1 - row] = source[row][col];
        return rotated;
    }

    // Kiểm tra mảnh có thể tồn tại tại (testX, testY) không.
    // Dùng trong getGhostY() để tìm điểm hạ cánh thấp nhất.
    bool canPieceExistAt(const Tetromino& tetromino, const Board& board,
                          int testX, int testY) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                if (!tetromino.isCellFilled(col, row)) continue;

                const int boardCol = testX + col;
                const int boardRow = testY + row;

                // Vượt biên trái, phải hoặc dưới → không hợp lệ.
                if (boardCol < 0 || boardCol >= BOARD_WIDTH || boardRow >= BOARD_HEIGHT)
                    return false;

                // boardRow < 0 được chấp nhận (mảnh mới xuất hiện một phần
                // phía trên bảng hiển thị).
                if (boardRow >= 0 && !board.isCellEmpty(boardCol, boardRow))
                    return false;
            }
        }
        return true;
    }
} // namespace

// ============================================================
// Constructor: khởi tạo mảnh tại vị trí xuất hiện mặc định.
// ============================================================
// x=3 đặt mảnh gần giữa bảng 10 cột; y=0 là hàng trên cùng.
Tetromino::Tetromino(TetrominoType type)
    : type(type)
    , rotation(0)
    , x(3)
    , y(0) {

    // Khởi tạo toàn bộ mảng shape về false trước.
    for (int rot = 0; rot < NUM_ROTATIONS; ++rot)
        for (int row = 0; row < TETROMINO_SIZE; ++row)
            for (int col = 0; col < TETROMINO_SIZE; ++col)
                shape[rot][row][col] = false;

    // Tính sẵn 4 trạng thái xoay bằng cách áp dụng rotateCWMatrix liên tiếp.
    Matrix4 current = getBaseShape(type);
    for (int rot = 0; rot < NUM_ROTATIONS; ++rot) {
        for (int row = 0; row < TETROMINO_SIZE; ++row)
            for (int col = 0; col < TETROMINO_SIZE; ++col)
                shape[rot][row][col] = current[row][col];
        current = rotateCWMatrix(current); // Chuẩn bị trạng thái xoay tiếp theo.
    }
}

// ── Truy vấn ─────────────────────────────────────────────────────────────────

TetrominoType Tetromino::getType() const     { return type; }
int           Tetromino::getRotation() const { return rotation; }

// Trả về true nếu ô (col, row) trong ma trận 4×4 ở góc xoay hiện tại có khối.
// Kiểm tra biên để tránh truy cập ngoài mảng.
bool Tetromino::isCellFilled(int col, int row) const {
    if (col < 0 || col >= TETROMINO_SIZE || row < 0 || row >= TETROMINO_SIZE)
        return false;
    return shape[rotation][row][col];
}

// ── Di chuyển ────────────────────────────────────────────────────────────────

void Tetromino::moveLeft()  { --x; }
void Tetromino::moveRight() { ++x; }
void Tetromino::moveDown()  { ++y; }
void Tetromino::moveUp()    { --y; } // Hoàn tác khi rơi vào vị trí không hợp lệ.

// ── Xoay ─────────────────────────────────────────────────────────────────────

// Xoay thuận chiều kim đồng hồ: tăng chỉ số rotation theo modulo 4.
void Tetromino::rotateCW()  { rotation = (rotation + 1) % NUM_ROTATIONS; }

// Xoay ngược chiều kim đồng hồ: +3 thay vì -1 để tránh số âm với modulo.
void Tetromino::rotateCCW() { rotation = (rotation + NUM_ROTATIONS - 1) % NUM_ROTATIONS; }

// ── Ghost Piece ───────────────────────────────────────────────────────────────

// Tìm hàng Y thấp nhất mà mảnh có thể tồn tại nếu rơi thẳng xuống.
// Renderer dùng giá trị này để vẽ bóng mờ cho người chơi.
int Tetromino::getGhostY(const Board& board) const {
    int ghostY = y;
    // Tiếp tục hạ xuống cho đến khi vị trí ghostY + 1 không còn hợp lệ.
    while (canPieceExistAt(*this, board, x, ghostY + 1)) {
        ++ghostY;
    }
    return ghostY;
}

// ── Bộ sinh 7-bag ────────────────────────────────────────────────────────────

// createRandom() dùng biến static để duy trì trạng thái túi (bag) xuyên suốt
// toàn bộ ván chơi. Mỗi khi túi rỗng, nó được nạp lại đủ 7 loại rồi xáo trộn.
// Thuật toán 7-bag đảm bảo người chơi không bao giờ phải đợi quá 12 mảnh
// để nhận một loại cụ thể (worst case: 7 mảnh cuối túi 1 + 5 mảnh đầu túi 2).
Tetromino Tetromino::createRandom() {
    static std::random_device rd;
    static std::mt19937 generator(rd()); // Mersenne Twister — phân phối đều
    static std::vector<TetrominoType> bag;

    if (bag.empty()) {
        // Nạp lại túi với đủ 7 loại mảnh.
        bag = {
            TetrominoType::I,
            TetrominoType::O,
            TetrominoType::T,
            TetrominoType::S,
            TetrominoType::Z,
            TetrominoType::J,
            TetrominoType::L
        };
        // Xáo trộn ngẫu nhiên.
        std::shuffle(bag.begin(), bag.end(), generator);
    }

    // Lấy mảnh từ cuối túi (pop_back O(1)).
    const TetrominoType nextType = bag.back();
    bag.pop_back();

    return Tetromino(nextType);
}

// ── Tiện ích cho Renderer ─────────────────────────────────────────────────────

// ID màu = giá trị int của enum TetrominoType (Renderer dùng để chọn màu).
int Tetromino::getColorID() const { return (int)type; }

// Tìm cột của khối thứ `index` trong mảnh (đếm theo thứ tự row-major).
int Tetromino::getBlockX(int index) const {
    int count = 0;
    for (int row = 0; row < TETROMINO_SIZE; ++row)
        for (int col = 0; col < TETROMINO_SIZE; ++col)
            if (shape[rotation][row][col]) {
                if (count == index) return col;
                ++count;
            }
    return 0;
}

// Tìm hàng của khối thứ `index` trong mảnh (đếm theo thứ tự row-major).
int Tetromino::getBlockY(int index) const {
    int count = 0;
    for (int row = 0; row < TETROMINO_SIZE; ++row)
        for (int col = 0; col < TETROMINO_SIZE; ++col)
            if (shape[rotation][row][col]) {
                if (count == index) return row;
                ++count;
            }
    return 0;
}
