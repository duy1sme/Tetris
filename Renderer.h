#pragma once 
//Yêu cầu máy tính chỉ nạp file này 1 lần duy nhất trong quá trình dịch code (tránh lặp lại).

#include <stdio.h> // Để dùng hàm sprintf
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
// để vẽ và sắp xếp không gian, để tải hình vào , để hiển thị điểm số người chơi.

#include"Board.h"
#include "Tetromino.h"//của thằng Huy

class Board;
class Tetromino;
//phòng tránh fixbug ở tv1 và tránh đọc vòng lặp//

class Renderer {
private:
    const int CELL_SIZE = 30;
    const int BOARD_X = 490; 
    const int BOARD_Y = 60;   
    
    // Khu vực bên trái 
    const int HOLD_X = BOARD_X - 150; 
    const int HOLD_Y = BOARD_Y + 50;  
    
    // Khu vực bên phải 
    const int NEXT_X = BOARD_X + 350; 
    const int NEXT_Y = BOARD_Y + 50;  
    const int SCORE_X = BOARD_X + 350;
    const int SCORE_Y = BOARD_Y + 400;



    SDL_Renderer* sdlRenderer;
    SDL_Texture* blockTexture;
    SDL_Texture* bgTexture;
    TTF_Font* mainFont;

    void drawBlock(int x, int y, int colorID, Uint8 alpha = 255);
    void drawText(const char* text, int x, int y, SDL_Color color);
    
    

public:

    Renderer(SDL_Renderer* renderer);
    ~Renderer();
    void drawLineClearEffect(int gridY, Uint8 alpha);

//load ảnh khối và ảnh background
    bool loadAssets() {
    blockTexture = IMG_LoadTexture(sdlRenderer, "block_green.png");
    
        if (!blockTexture) {

            printf("Loi nap anh: %s\n", SDL_GetError());
            return false;
    }
        bgTexture = IMG_LoadTexture(sdlRenderer, "bg.png");
        if (!bgTexture) {
            printf("Loi nap anh nen: %s\n", SDL_GetError());
        }
        
        return true; 
    
}

    
    void renderGame(const Board& board, const Tetromino& currentPiece, 
                    const Tetromino& nextPiece, int score, int level, int state);
};


void Renderer::drawBlock(int gridX, int gridY, int colorID, Uint8 alpha) {
    if (colorID == 0) return;   

    int pixel_X = BOARD_X + (gridX * CELL_SIZE);
    int pixel_Y = BOARD_Y + (gridY * CELL_SIZE);

    SDL_Rect dest = { pixel_X, pixel_Y, CELL_SIZE, CELL_SIZE };
    // Bảng quy đổi mã màu 
    SDL_Color c = {255, 255, 255, 255};
    switch (colorID) {
        case 1: c = {0, 255, 255, 255}; break;   // Xanh lơ 
        case 2: c = {0, 0, 255, 255}; break;     // Xanh dương 
        case 3: c = {255, 165, 0, 255}; break;   // Cam 
        case 4: c = {255, 255, 0, 255}; break;   // Vàng 
        case 5: c = {0, 255, 0, 255}; break;     // Xanh lá
        case 6: c = {128, 0, 128, 255}; break;   // Tím
        case 7: c = {255, 0, 0, 255}; break;     // Đỏ 
        case 8: c = {150, 150, 150, 255}; break; // Xám 
    }

    if (blockTexture) {
        SDL_SetTextureColorMod(blockTexture, c.r, c.g, c.b); 
        SDL_SetTextureAlphaMod(blockTexture, alpha);        
        SDL_RenderCopy(sdlRenderer, blockTexture, NULL, &dest); 
    }
}
void Renderer::drawText(const char* text, int x, int y, SDL_Color color) {
    if (!mainFont) return; // Nếu chưa nạp file font thì bỏ qua việc vẽ chữ

    // Khắc chữ lên bề mặt
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(mainFont, text, color);
    if (!surfaceMessage) return;
    
    // chuyển bề mặt về 2d để card màn hình xử lý
    SDL_Texture* Message = SDL_CreateTextureFromSurface(sdlRenderer, surfaceMessage);
    
    // Lấy kích thước thật của dòng chữ
    SDL_Rect Message_rect;
    Message_rect.x = x;  
    Message_rect.y = y; 
    Message_rect.w = surfaceMessage->w; 
    Message_rect.h = surfaceMessage->h;

    // Dán bức ảnh chữ đó lên màn hình chính
    SDL_RenderCopy(sdlRenderer, Message, NULL, &Message_rect);
    
    // Dọn dẹp bộ nhớ
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

void Renderer::renderGame(const Board& board, const Tetromino& currentPiece, 
                          const Tetromino& nextPiece, int score, int level, int state) {
    
    //background
    if (bgTexture) {
        SDL_RenderCopy(sdlRenderer, bgTexture, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 30, 255);
        SDL_RenderClear(sdlRenderer);
    }

//vẽ khung 
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            int cellColor = board.grid[y][x]; 
            if (cellColor != 0) {
                drawBlock(x, y, cellColor, 255);
            }
        }
    }

// khối hiện tại và điểm rơi của nó
    // xác định điểm rơi
    int ghostY = currentPiece.y;
    while (board.isValidPosition(currentPiece, currentPiece.x, ghostY + 1)) {
        ghostY++;
    }

    // điểm rơi
    for (int i = 0; i < 4; i++) {
        int px = currentPiece.x + currentPiece.blocks[i].x;
        int py = ghostY + currentPiece.blocks[i].y;
        // Dùng .type thay vì getColor()
        drawBlock(px, py, currentPiece.type, 80); 
    }
    
    // Vẽ Khối gạch
    for (int i = 0; i < 4; i++) {
        int px = currentPiece.x + currentPiece.blocks[i].x;
        int py = currentPiece.y + currentPiece.blocks[i].y;
        drawBlock(px, py, currentPiece.type, 255); 
    }

//giao diện 2 bên
    SDL_Color white = {255, 255, 255, 255};
    char textBuffer[64]; 

    // Khung BÊN PHẢI 
    drawText("NEXT", NEXT_X, NEXT_Y - 40, white);
    for (int i = 0; i < 4; i++) {
        int px = (NEXT_X - BOARD_X) / CELL_SIZE + nextPiece.blocks[i].x;
        int py = (NEXT_Y - BOARD_Y) / CELL_SIZE + nextPiece.blocks[i].y;
        drawBlock(px, py, nextPiece.type, 255);
    }

    sprintf(textBuffer, "SCORE: %d", score);
    drawText(textBuffer, SCORE_X, SCORE_Y, white);

    // Khung BÊN TRÁI 

    drawText("HOLD", HOLD_X, HOLD_Y - 40, white);
    
    sprintf(textBuffer, "LEVEL: %d", level); 
    drawText(textBuffer, HOLD_X, HOLD_Y + 350, white); 


//hiển thị
    SDL_RenderPresent(sdlRenderer);
}
// Hàm tạo hiệu ứng 
void Renderer::drawLineClearEffect(int gridY, Uint8 alpha) {
    int pixel_Y = BOARD_Y + (gridY * CELL_SIZE);
    
    //hiệu ứng hiển thị
    SDL_Rect flashRect = { BOARD_X, pixel_Y, 10 * CELL_SIZE, CELL_SIZE };
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, alpha); 
    
    // Vẽ đè lên màn hình
    SDL_RenderFillRect(sdlRenderer, &flashRect);
}


