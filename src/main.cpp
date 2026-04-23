#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> 
/*SDL_main.h cần thiết trên Windows để SDL thay thế hàm main() của bạn thành WinMain 
bên dưới — không có cái này thì có thể bị lỗi linker trên một số cấu hình.*/ 
#include <iostream>
int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        /*SDL_INIT_VIDEO — bật hệ thống cửa sổ & vẽ hình
        SDL_INIT_AUDIO — bật hệ thống âm thanh (TV4 cần sau)
        Nếu thất bại thì SDL_GetError() trả về lý do, in ra rồi thoát*/
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Khởi động SDL
    SDL_Window* window = SDL_CreateWindow(
        "Tetris",   // tiêu đề
        640,        // width
        480,         // height
        0           // flags
    );

    // Tạo cửa sổ
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Tạo Renderer để vẽ lên cửa sổ
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Vòng lặp chờ thoát
    bool running = true;
    SDL_Event event;

    while (running) {
        // Xử lý event
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Tô màu nền (tạm dùng màu đen)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    // Dọn dẹp 
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}