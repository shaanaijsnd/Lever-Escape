#ifndef _GAME_GRAPHICS_H_
#define _GAME_GRAPHICS_H_

#include <SFML/Graphics.hpp>
#include <vector>
#include "gameLogic.hpp"

#define LOAD_GRAPHICS_ERROR "Error when loading: "
#define LOAD_TILESET "sprites/tileset1.png"
#define LOAD_EL_LOCO "sprites/elLoco.png"
#define LOAD_FONT "fonts/mismo/Mismo Black.ttf"
#define TITLE "Escape El Loco"
#define END_TEXT "Congratulations!!\nYou managed to escape El Loco's Tower!\n\nBeware, he may return..."
#define THANKS_TEXT "Thank you for playing my game. :)"
#define LEFT_CENTER_X 512
#define CENTER_Y 1408
#define FRAMERATE_FONT_SIZE 24
#define END_FONT_SIZE 32
#define THANKS_FONT_SIZE 16
#define FRAMERATE 60
#define TILE_DIM 64
#define EMPTY 98
#define ROTATION 25
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Escape El Loco"
#define FRAMERATE_LIMIT 60

class GameGraphics {
public:
	int left_center_x;
	int bound_min_y;
	int bound_max_y;
	int bound_x1;
	int bound_x2;
	bool hardcoreMode, end;
	GameLogic& gameLogic;
	sf::RenderWindow window;
	sf::Text framerateText, endText;
	sf::Font font;
	sf::Color color;
	sf::Texture background;
	sf::Texture playerTexture;
	sf::Sprite elLocoSprite;
	sf::Texture elLocoTexture;
	std::vector<std::vector<sf::Sprite>> map;
	sf::View viewLeft;
	sf::RectangleShape border;
	float framerate, blinkTime, rotation;


	GameGraphics(GameLogic& gl);
	void init();
	void reinit();
	void checkUpdate();
	void update(float deltaTime);
	void load();
	void loadMap();
	void drawBackground(int k);
	void displayEndGame();
	void buildWindow();
	float clamp(float v, float min, float max) const;
	~GameGraphics();
};

#endif