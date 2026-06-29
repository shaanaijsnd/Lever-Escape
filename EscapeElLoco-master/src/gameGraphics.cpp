#include "gameGraphics.hpp"
#include <math.h>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace sf;

GameGraphics::GameGraphics(GameLogic& gl) : hardcoreMode(false), end(false), gameLogic(gl), rotation(0)
{
    init();
}

void GameGraphics::displayEndGame()
{
    sf::Text text;

    text.setFont(font);
    text.setCharacterSize(60);
    text.setFillColor(sf::Color::White);
    text.setString("YOU WIN!");

    sf::FloatRect r = text.getLocalBounds();

    text.setOrigin(r.width / 2.f, r.height / 2.f);
    text.setPosition(
        WINDOW_WIDTH / 2.f,
        WINDOW_HEIGHT / 2.f
    );

    window.clear(sf::Color::Black);
    window.draw(text);
    window.display();
}

void GameGraphics::drawBackground(int)
{
    for (size_t i = 0; i < map.size(); i++)
    {
        for (size_t j = 0; j < map[i].size(); j++)
        {
            window.draw(map[i][j]);
        }
    }
}

void GameGraphics::checkUpdate()
{
    for (auto& m : gameLogic.modifs)
    {
        map[m.y][m.x].setTextureRect(
            sf::IntRect(
                (m.value % 8) * 16,
                (m.value / 8) * 16,
                16,
                16
            )
        );
    }
}

float GameGraphics::clamp(float value, float min, float max) const
{
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

void GameGraphics::buildWindow()
{
    window.create(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        WINDOW_TITLE,
        sf::Style::Close
    );

    window.setFramerateLimit(FRAMERATE_LIMIT);
}

void GameGraphics::init()
{
    buildWindow();

    load();

    framerate = 0.f;

    framerateText.setFont(font);
    framerateText.setCharacterSize(20);
    framerateText.setFillColor(sf::Color::White);
    framerateText.setPosition(10.f, 10.f);

    viewLeft = window.getDefaultView();

    left_center_x = viewLeft.getSize().x / 2.f;
    bound_min_y = viewLeft.getSize().y / 2.f;
}

void GameGraphics::loadMap() {
    std::string sprite;



    if (gameLogic.level % 2)
    {
        sprite = "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/sprites/dino/3.png";
    }
    else
    {
        sprite = "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/sprites/dino/1.png";
    }

    std::cout << "Loading sprite: " << sprite << std::endl;

    if (!std::filesystem::exists(sprite))
    {
        std::cerr << "File not found: " << sprite << std::endl;
        exit(1);
    }



    std::cout << "Loading sprite: " << sprite << std::endl;

    std::cout << "File exists: "
        << (std::filesystem::exists(sprite) ? "YES" : "NO")
        << std::endl;
    std::cout << "File exists = "
        << std::filesystem::exists(sprite)
        << std::endl;
    if (!playerTexture.loadFromFile(sprite))
    {
        std::cerr << "ERROR: Cannot load sprite file: "
            << sprite << std::endl;
        exit(1);
    }


    sf::Sprite s;
    map.emplace_back();

    for (int i = 0; i < gameLogic.map.height; i++) {
        for (int j = 0; j < gameLogic.map.width; j++) {
            s.setTexture(background);

            int tileNumber = gameLogic.map.map[i][j];

            if (tileNumber == TABLE) {
                s.setTextureRect(IntRect((tileNumber % 8) << 4,
                    ((tileNumber >> 3) << 4) + 8,
                    16, 8));
                s.setPosition(j << 6, (i << 6) + 32);
            }
            else {
                if (tileNumber == COLLAPSE_BLOCK)
                    s.setTextureRect(IntRect((tileNumber % 8) << 4,
                        (tileNumber >> 3) << 4,
                        16, 8));
                else
                    s.setTextureRect(IntRect((tileNumber % 8) << 4,
                        (tileNumber >> 3) << 4,
                        16, 16));

                s.setPosition(j << 6, i << 6);
            }

            s.setScale(4.f, 4.f);
            map.back().push_back(s);
        }

        map.emplace_back();
    }

    gameLogic.player.setSprite(playerTexture);

    Vector2f size = viewLeft.getSize();

    bound_min_y = size.y / 2;
    bound_max_y = gameLogic.map.height * 64 - bound_min_y;

    left_center_x = size.x / 2;

    // Camera được phép chạy hết bản đồ
    bound_x1 = gameLogic.map.width * 64 - left_center_x;
    bound_x2 = bound_x1;
}


void GameGraphics::load()
{
    std::string file =
        "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/sprites/tileset1.png";

    std::ifstream test(file, std::ios::binary);

    if (test.is_open())
        std::cout << "FILE OPEN OK" << std::endl;
    else
    {
        std::cout << "FILE OPEN FAIL" << std::endl;
        exit(1);
    }

    std::cout << "Current path = "
        << std::filesystem::current_path()
        << std::endl;

    std::cout << "Loading file: [" << file << "]" << std::endl;

    sf::Image img;

    if (img.loadFromFile(file))
    {
        std::cout << "IMAGE LOAD OK" << std::endl;
    }
    else
    {
        std::cout << "IMAGE LOAD FAIL" << std::endl;
        exit(1);
    }

    // LOAD TEXTURE CHO MAP
    if (!background.loadFromFile(file))
    {
        std::cout << "BACKGROUND LOAD FAIL" << std::endl;
        exit(1);
    }
    else
    {
        std::cout << "BACKGROUND LOAD OK" << std::endl;
    }

    if (!font.loadFromFile(LOAD_FONT))
    {
        std::cerr << LOAD_GRAPHICS_ERROR
            << LOAD_FONT << std::endl;
        exit(1);
    }

    if (!elLocoTexture.loadFromFile(LOAD_EL_LOCO))
    {
        std::cerr << LOAD_GRAPHICS_ERROR
            << LOAD_EL_LOCO << std::endl;
        exit(1);
    }

    elLocoSprite.setTexture(elLocoTexture);
    elLocoSprite.setOrigin(32, 32);
    gameLogic.elLoco = elLocoSprite;

    loadMap();
}
void GameGraphics::update(float deltaTime) {
    if (gameLogic.level > 2) {
        if (!end) {
            end = true;
            displayEndGame();
        }
        return;
    }
    framerate = 1.f / (deltaTime);
    framerateText.setString(std::to_string((int)ceil(framerate)) + " fps");

    if (hardcoreMode != gameLogic.hardcoreMode) {
        if (hardcoreMode) {
            rotation = 0;
            viewLeft.setRotation(0);
        }
        hardcoreMode = !hardcoreMode;
    }
    if (hardcoreMode) {
        viewLeft.setRotation(-rotation);
        rotation += ROTATION * deltaTime;
    }

    checkUpdate();

    //Draw
    viewLeft.setCenter(
        clamp(
            gameLogic.player.x,
            left_center_x,
            bound_x1
        ),
        clamp(
            gameLogic.player.y,
            bound_min_y,
            bound_max_y
        )
    );

    window.setView(window.getDefaultView());
    window.clear(Color::Black);

    window.setView(viewLeft);
    drawBackground(0);
    window.draw(gameLogic.player.sprite);

    window.setView(window.getDefaultView());
    window.draw(framerateText);

    window.display();
}

GameGraphics::~GameGraphics() {}