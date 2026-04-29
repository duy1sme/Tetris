#include "Tetromino.h"
#include "Board.h"

#include <algorithm>
#include <array>
#include <random>
#include <vector>

// ==========================
// File Tetromino.cpp
// Nhiệm vụ: Quản lý logic của các mảnh Tetris.
// File này xử lý hình dạng 7 mảnh, trạng thái xoay, di chuyển,
// ghost piece và sinh mảnh ngẫu nhiên.
// ==========================

// namespace ẩn danh: các hàm bên trong chỉ dùng trong file Tetromino.cpp.
// Như vậy tránh bị trùng tên với các file khác trong project.
namespace {
    // Matrix4 là ma trận 4x4 kiểu bool.
    // true  = ô có khối.
    // false = ô trống.
    using Matrix4 = std::array<std::array<bool, TETROMINO_SIZE>, TETROMINO_SIZE>;

    // emptyMatrix(): Tạo ma trận 4x4 rỗng.
    // Mặc dù std::array mặc định có thể khởi tạo false,
    // nhưng viết rõ vòng lặp giúp dễ hiểu khi trình bày với thầy.
    Matrix4 emptyMatrix() {
        Matrix4 matrix{};
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                matrix[row][col] = false;
            }
        }
        return matrix;
    }

    // getBaseShape(): Trả về hình dạng ban đầu của từng loại mảnh Tetris.
    // Các mảnh gồm: I, O, T, S, Z, J, L.
    // Mỗi mảnh được biểu diễn trong ma trận 4x4.
    Matrix4 getBaseShape(TetrominoType type) {
        Matrix4 shape = emptyMatrix();

        switch (type) {
            // Mảnh I: dạng thanh ngang 4 ô.
            case TetrominoType::I:
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                shape[1][3] = true;
                break;

            // Mảnh O: hình vuông 2x2.
            case TetrominoType::O:
                shape[0][1] = true;
                shape[0][2] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            // Mảnh T.
            case TetrominoType::T:
                shape[0][1] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            // Mảnh S.
            case TetrominoType::S:
                shape[0][1] = true;
                shape[0][2] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                break;

            // Mảnh Z.
            case TetrominoType::Z:
                shape[0][0] = true;
                shape[0][1] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            // Mảnh J.
            case TetrominoType::J:
                shape[0][0] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            // Mảnh L.
            case TetrominoType::L:
                shape[0][2] = true;
                shape[1][0] = true;
                shape[1][1] = true;
                shape[1][2] = true;
                break;

            // NONE không có hình dạng.
            case TetrominoType::NONE:
            default:
                break;
        }

        return shape;
    }

    // rotateCWMatrix(): Xoay một ma trận 4x4 theo chiều kim đồng hồ.
    // Công thức xoay:
    // ô cũ [row][col] sẽ chuyển sang ô mới [col][size - 1 - row].
    Matrix4 rotateCWMatrix(const Matrix4& source) {
        Matrix4 rotated = emptyMatrix();

        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                rotated[col][TETROMINO_SIZE - 1 - row] = source[row][col];
            }
        }

        return rotated;
    }

    // canPieceExistAt(): Kiểm tra xem mảnh có thể tồn tại tại vị trí testX, testY không.
    // Hàm này dùng cho ghost piece để thử cho mảnh rơi xuống thấp nhất.
    // Nếu mảnh vượt biên hoặc đè lên ô đã có khối thì trả về false.
    bool canPieceExistAt(const Tetromino& tetromino, const Board& board, int testX, int testY) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                // Bỏ qua các ô trống của mảnh.
                if (!tetromino.isCellFilled(col, row)) {
                    continue;
                }

                // Chuyển tọa độ trong ma trận 4x4 sang tọa độ trên board.
                const int boardCol = testX + col;
                const int boardRow = testY + row;

                // Không cho mảnh vượt trái, phải hoặc vượt đáy board.
                if (boardCol < 0 || boardCol >= BOARD_WIDTH || boardRow >= BOARD_HEIGHT) {
                    return false;
                }

                // Nếu ô nằm trong board và không trống thì mảnh không thể đặt ở đây.
                // Điều kiện boardRow >= 0 cho phép mảnh có thể xuất hiện một phần phía trên board lúc mới sinh.
                if (boardRow >= 0 && !board.isCellEmpty(boardCol, boardRow)) {
                    return false;
                }
            }
        }

        return true;
    }
}

// Constructor Tetromino:
// Tạo một mảnh mới với loại type truyền vào.
// Mảnh bắt đầu ở gần giữa bàn chơi: x = 3, y = 0.
// rotation = 0 nghĩa là trạng thái xoay ban đầu.
Tetromino::Tetromino(TetrominoType type)
    : type(type)
    , rotation(0)
    , x(3)
    , y(0) {

    // Xóa toàn bộ dữ liệu shape trước khi gán hình dạng thật.
    for (int rot = 0; rot < NUM_ROTATIONS; ++rot) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                shape[rot][row][col] = false;
            }
        }
    }

    // Lấy hình dạng ban đầu của mảnh.
    Matrix4 current = getBaseShape(type);

    // Tạo sẵn 4 trạng thái xoay cho mảnh.
    // shape[0] là trạng thái ban đầu.
    // shape[1], shape[2], shape[3] là các trạng thái sau khi xoay tiếp 90 độ.
    for (int rot = 0; rot < NUM_ROTATIONS; ++rot) {
        for (int row = 0; row < TETROMINO_SIZE; ++row) {
            for (int col = 0; col < TETROMINO_SIZE; ++col) {
                shape[rot][row][col] = current[row][col];
            }
        }

        current = rotateCWMatrix(current);
    }
}

// getType(): Trả về loại mảnh hiện tại.
// Board dùng để biết khi lockPiece thì phải ghi loại mảnh nào vào grid.
TetrominoType Tetromino::getType() const {
    return type;
}

// getRotation(): Trả về trạng thái xoay hiện tại của mảnh.
int Tetromino::getRotation() const {
    return rotation;
}

// isCellFilled(): Kiểm tra một ô trong ma trận 4x4 của mảnh có khối hay không.
// col là cột trong mảnh, row là hàng trong mảnh.
// Hàm này dùng cho render, kiểm tra va chạm và lockPiece.
bool Tetromino::isCellFilled(int col, int row) const {
    if (col < 0 || col >= TETROMINO_SIZE || row < 0 || row >= TETROMINO_SIZE) {
        return false;
    }

    return shape[rotation][row][col];
}

// moveLeft(): Di chuyển mảnh sang trái 1 ô.
// Hàm chỉ thay đổi tọa độ, việc kiểm tra có được đi hay không do Game/Board xử lý.
void Tetromino::moveLeft() {
    --x;
}

// moveRight(): Di chuyển mảnh sang phải 1 ô.
void Tetromino::moveRight() {
    ++x;
}

// moveDown(): Di chuyển mảnh xuống dưới 1 ô.
void Tetromino::moveDown() {
    ++y;
}

// rotateCW(): Xoay mảnh theo chiều kim đồng hồ.
// Vì có 4 trạng thái xoay nên dùng % NUM_ROTATIONS để quay vòng từ 3 về 0.
void Tetromino::rotateCW() {
    rotation = (rotation + 1) % NUM_ROTATIONS;
}

// rotateCCW(): Xoay mảnh ngược chiều kim đồng hồ.
// Cộng NUM_ROTATIONS - 1 để tránh bị số âm.
void Tetromino::rotateCCW() {
    rotation = (rotation + NUM_ROTATIONS - 1) % NUM_ROTATIONS;
}

// getGhostY(): Tính vị trí y thấp nhất mà mảnh có thể rơi tới.
// Phần Renderer có thể dùng ghostY để vẽ bóng mờ của mảnh.
// Hàm thử tăng y liên tục cho đến khi mảnh không thể xuống thêm.
int Tetromino::getGhostY(const Board& board) const {
    int ghostY = y;

    while (canPieceExistAt(*this, board, x, ghostY + 1)) {
        ++ghostY;
    }

    return ghostY;
}

// createRandom(): Sinh mảnh Tetris ngẫu nhiên.
// Ở đây dùng thuật toán 7-bag randomizer:
// - Một túi có đủ 7 mảnh I, O, T, S, Z, J, L.
// - Trộn ngẫu nhiên túi.
// - Mỗi lần lấy ra 1 mảnh.
// - Khi túi hết thì tạo túi mới.
// Cách này giúp các mảnh xuất hiện đều hơn so với random hoàn toàn.
Tetromino Tetromino::createRandom() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::vector<TetrominoType> bag;

    if (bag.empty()) {
        bag = {
            TetrominoType::I,
            TetrominoType::O,
            TetrominoType::T,
            TetrominoType::S,
            TetrominoType::Z,
            TetrominoType::J,
            TetrominoType::L
        };

        std::shuffle(bag.begin(), bag.end(), generator);
    }

    const TetrominoType nextType = bag.back();
    bag.pop_back();

    return Tetromino(nextType);
}

int Tetromino::getColorID() const {
    return (int)type;
}

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

void Tetromino::moveUp() { --y; }