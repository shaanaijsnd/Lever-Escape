#include <SFML/Audio.hpp>
#include <iostream>
#include "game.hpp"

using namespace sf;
using namespace std;

Game::Game() : gameAudio(),gameLogic(gameAudio),gameGraphics(gameLogic) {
    run();
}

void Game::run() {
    // Vòng lặp ngoài: cho phép quay lại menu và chơi lại
    while (gameGraphics.window.isOpen())
    {
        // Hiện menu chính
        int menuChoice = gameGraphics.showMenu();
        if (menuChoice != 0 || !gameGraphics.window.isOpen())
            return; // Thoát hẳn

        // Reset game cho lần chơi mới
        gameLogic.resetGame();
        gameGraphics.reinit();
        gameAudio.load();

        deltaTime = 1.0f / MINUTE;
        Clock clock;
        Event event;
        bool backToMenu = false;
        bool escPressed = false;

        // ── Vòng lặp game chính ────────────────────────────
        while (gameGraphics.window.isOpen() && gameLogic.level <= 2 && !backToMenu)
        {
            int keyCode = -1;
            escPressed = false;
            while (gameGraphics.window.pollEvent(event)) {
                if (event.type == Event::Closed)
                { gameGraphics.window.close(); return; }
                else if (event.type == Event::LostFocus)  gameLogic.pause = true;
                else if (event.type == Event::GainedFocus) gameLogic.pause = false;
                else if (event.type == Event::KeyReleased) {
                    if (event.key.code == Keyboard::Escape) escPressed = true;
                    else keyCode = event.key.code;
                }
            }

            // ESC bấm (không giữ) → hiện menu tạm dừng
            if (escPressed)
            {
                gameLogic.pause = true;
                int pauseResult = gameGraphics.showPause();
                // 0 = Tiếp tục, 1 = Về menu, 2 = Thoát hẳn
                if (pauseResult == 2)
                { gameGraphics.window.close(); return; }
                else if (pauseResult == 1)
                { backToMenu = true; break; }
                else
                {
                    gameLogic.pause = false;
                    clock.restart(); // tránh deltaTime cộng dồn thời gian pause
                    continue;
                }
            }

            gameLogic.update(deltaTime, gameGraphics.map, keyCode);
            gameGraphics.update(deltaTime);

            deltaTime = clock.getElapsedTime().asSeconds();
            clock.restart();
        }

        // Nếu về menu thì vòng lặp ngoài sẽ hiện lại showMenu()
        // Nếu win (level > 2) thì hiện màn chiến thắng rồi về menu
        if (!backToMenu && gameGraphics.window.isOpen() && gameLogic.level > 2)
        {
            gameGraphics.displayEndGame();
            // Đợi người chơi nhấn phím bất kỳ rồi về menu
            bool waiting = true;
            while (gameGraphics.window.isOpen() && waiting)
            {
                while (gameGraphics.window.pollEvent(event)) {
                    if (event.type == Event::Closed)
                    { gameGraphics.window.close(); return; }
                    if (event.type == Event::KeyReleased || event.type == Event::MouseButtonReleased)
                        waiting = false;
                }
            }
        }
    }
}

Game::~Game(){}
