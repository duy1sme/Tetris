#include "Game.h"
#include <iostream>

Game::Game()
    : window(nullptr)
    , sdlRenderer(nullptr)
    , state(GameState::MENU)
    , running(false)
    , currentPiece(nullptr)
    , nextPiece(nullptr)
    , renderer(nullptr)
    , score(0)
    , level(1)
    , totalLines(0)
    , fallTimer(0.0f)
    , fallInterval(1.0f) {
}

bool Game::init() {
    // Khởi động SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Tạo cửa sổ
    window = SDL_CreateWindow("Tetris", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Tạo SDL Renderer
    sdlRenderer = SDL_CreateRenderer(window, NULL);
    if (!sdlRenderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Tạo Renderer class
    renderer = new Renderer(sdlRenderer);

    // Khởi động audio
    audio.init();

    // Spawn mảnh đầu tiên
    // spawnPiece();  ← comment lại, TV1 chưa làm xong

    running = true;
    state   = GameState::PLAYING;
    return true;
}

void Game::run() {
    Uint64 lastTime = SDL_GetTicks();

    while (running) {
        // Tính deltaTime
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime    = (currentTime - lastTime) / 1000.0f;
        lastTime           = currentTime;

        handleInput();
        update(deltaTime);
        render();
    }
}

void Game::shutdown() {
    delete currentPiece;
    delete nextPiece;
    delete renderer;

    audio.shutdown();

    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

Game::~Game() {
    // Đảm bảo cleanup nếu ai đó quên gọi shutdown()
    if (running) {
        shutdown();
    }
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }
}

void Game::update(float deltaTime) {
    // TODO: logic game
}

void Game::render() {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
}

void Game::spawnPiece()                                  { /* TV1 */ }
bool Game::isValidPosition(const Tetromino& piece) const { return true; }
void Game::lockCurrentPiece()                            { /* TV1 */ }
void Game::calculateScore(int linesCleared)              { /* TODO */ }
void Game::changeState(GameState newState)               { state = newState; }