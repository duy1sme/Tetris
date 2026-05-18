/*
 * GameInput.cpp — Xử lý toàn bộ input (chuột + bàn phím) cho mọi GameState.
 */
#include "Game.h"
#include <iostream>

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        audio.handleEvent(event);

        // ── Thoát ────────────────────────────────────────────
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
            return;
        }

        // ── Mouse Motion — hover tracking ────────────────────
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            float mx = event.motion.x;
            float my = event.motion.y;

            if (state == GameState::MENU) {
                menuHoveredBtn = -1;
                if (isMouseInRect(mx, my, btnMenuPlay))     menuHoveredBtn = 0;
                if (isMouseInRect(mx, my, btnMenuSettings)) menuHoveredBtn = 1;
                if (isMouseInRect(mx, my, btnMenuTutorial)) menuHoveredBtn = 2;
            }
            else if (state == GameState::LEVEL_SELECT) {
                levelHovered = -1;
                for (int i = 0; i < NUM_LEVELS; i++)
                    if (isMouseInRect(mx, my, btnLevelCards[i])) levelHovered = i;
                if (isMouseInRect(mx, my, btnLevelBack)) levelHovered = NUM_LEVELS;
            }
            else if (state == GameState::SETTINGS) {
                settingsHovered = -1;
                if (isMouseInRect(mx, my, bgmSliderRect)) settingsHovered = 0;
                if (isMouseInRect(mx, my, bgmToggleRect)) settingsHovered = 1;
                if (isMouseInRect(mx, my, sfxSliderRect)) settingsHovered = 2;
                if (isMouseInRect(mx, my, sfxToggleRect)) settingsHovered = 3;
                for (int i = 0; i < 7; i++)
                    if (isMouseInRect(mx, my, keyBindRects[i])) settingsHovered = 4 + i;
                if (isMouseInRect(mx, my, btnSettingsBack)) settingsHovered = 11;

                // Drag sliders
                if (draggingBGM) {
                    bgmVolume = std::max(0.0f, std::min(1.0f,
                        (mx - bgmSliderRect.x) / bgmSliderRect.w));
                    audio.setBGMVolume(bgmVolume);
                }
                if (draggingSFX) {
                    sfxVolume = std::max(0.0f, std::min(1.0f,
                        (mx - sfxSliderRect.x) / sfxSliderRect.w));
                    audio.setSFXVolume(sfxVolume);
                }
            }
        }

        // ── Mouse Button Up — stop dragging ──────────────────
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            draggingBGM = false;
            draggingSFX = false;
        }

        // ── Mouse Click ──────────────────────────────────────
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT) {
            float mx = event.button.x;
            float my = event.button.y;

            switch (state) {
            case GameState::MENU:
                if (isMouseInRect(mx, my, btnMenuPlay))
                    changeState(GameState::LEVEL_SELECT);
                else if (isMouseInRect(mx, my, btnMenuSettings))
                    changeState(GameState::SETTINGS);
                else if (isMouseInRect(mx, my, btnMenuTutorial))
                    changeState(GameState::TUTORIAL);
                break;

            case GameState::LEVEL_SELECT:
                for (int i = 0; i < NUM_LEVELS; i++) {
                    if (isMouseInRect(mx, my, btnLevelCards[i])) {
                        selectedLevel = i;
                        startGame(i);
                    }
                }
                if (isMouseInRect(mx, my, btnLevelBack))
                    changeState(GameState::MENU);
                break;

            case GameState::SETTINGS:
                // BGM slider click
                if (isMouseInRect(mx, my, bgmSliderRect)) {
                    draggingBGM = true;
                    bgmVolume = std::max(0.0f, std::min(1.0f,
                        (mx - bgmSliderRect.x) / bgmSliderRect.w));
                    audio.setBGMVolume(bgmVolume);
                }
                // SFX slider click
                if (isMouseInRect(mx, my, sfxSliderRect)) {
                    draggingSFX = true;
                    sfxVolume = std::max(0.0f, std::min(1.0f,
                        (mx - sfxSliderRect.x) / sfxSliderRect.w));
                    audio.setSFXVolume(sfxVolume);
                }
                // BGM toggle
                if (isMouseInRect(mx, my, bgmToggleRect))
                    audio.toggleBGM();
                // SFX toggle
                if (isMouseInRect(mx, my, sfxToggleRect))
                    audio.toggleSFX();
                // Key bindings
                for (int i = 0; i < 7; i++) {
                    if (isMouseInRect(mx, my, keyBindRects[i])) {
                        editingKeyIndex = i;
                    }
                }
                // Back
                if (isMouseInRect(mx, my, btnSettingsBack)) {
                    editingKeyIndex = -1;
                    changeState(GameState::MENU);
                }
                break;

            case GameState::TUTORIAL:
                if (isMouseInRect(mx, my, btnTutorialBack))
                    changeState(GameState::MENU);
                break;

            case GameState::GAME_OVER:
                if (isMouseInRect(mx, my, btnGORestart))
                    startGame(selectedLevel);
                else if (isMouseInRect(mx, my, btnGOMenu))
                    changeState(GameState::MENU);
                break;

            default: break;
            }
        }

        // ── Keyboard ─────────────────────────────────────────
        if (event.type == SDL_EVENT_KEY_DOWN) {
            SDL_Keycode key = event.key.key;

            // Settings: key rebinding mode
            if (state == GameState::SETTINGS && editingKeyIndex >= 0) {
                if (key == SDLK_ESCAPE) {
                    editingKeyIndex = -1; // Cancel
                } else {
                    SDL_Keycode* bindings[] = {
                        &keys.moveLeft, &keys.moveRight, &keys.softDrop,
                        &keys.hardDrop, &keys.rotateCW, &keys.rotateCCW, &keys.hold
                    };
                    *bindings[editingKeyIndex] = key;
                    editingKeyIndex = -1;
                }
                continue;
            }

            // ESC: pause toggle
            if (key == SDLK_ESCAPE) {
                if (state == GameState::PLAYING)
                    changeState(GameState::PAUSED);
                else if (state == GameState::PAUSED)
                    changeState(GameState::PLAYING);
                else if (state == GameState::SETTINGS)
                    changeState(GameState::MENU);
                else if (state == GameState::TUTORIAL)
                    changeState(GameState::MENU);
                else if (state == GameState::LEVEL_SELECT)
                    changeState(GameState::MENU);
            }

            // ENTER shortcuts
            if (key == SDLK_RETURN) {
                if (state == GameState::MENU)
                    changeState(GameState::LEVEL_SELECT);
                else if (state == GameState::GAME_OVER)
                    changeState(GameState::MENU);
            }

            // Playing controls
            if (state != GameState::PLAYING || currentPiece == nullptr)
                continue;

            if (key == keys.moveLeft) {
                currentPiece->moveLeft();
                if (!isValidPosition(*currentPiece)) currentPiece->moveRight();
                else audio.playSFX(SoundType::MOVE);
            }
            else if (key == keys.moveRight) {
                currentPiece->moveRight();
                if (!isValidPosition(*currentPiece)) currentPiece->moveLeft();
                else audio.playSFX(SoundType::MOVE);
            }
            else if (key == keys.softDrop) {
                currentPiece->moveDown();
                if (!isValidPosition(*currentPiece)) {
                    currentPiece->moveUp();
                    lockCurrentPiece();
                } else {
                    score += 1;
                    audio.playSFX(SoundType::MOVE);
                }
            }
            else if (key == keys.rotateCW) {
                currentPiece->rotateCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCCW();
                else audio.playSFX(SoundType::ROTATE);
            }
            else if (key == keys.rotateCCW) {
                currentPiece->rotateCCW();
                if (!isValidPosition(*currentPiece)) currentPiece->rotateCW();
                else audio.playSFX(SoundType::ROTATE);
            }
            else if (key == keys.hold) {
                holdPiece();
            }
            else if (key == keys.hardDrop) {
                hardDrop();
            }
        }
    }
}
