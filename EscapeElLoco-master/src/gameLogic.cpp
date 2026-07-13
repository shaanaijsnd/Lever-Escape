#include <fstream>
#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>
#include "gameLogic.hpp"

using namespace std;

GameLogic::GameLogic(GameAudio& audio)
	: gameAudio(audio),
	level(1),
	pause(false),
	end(false),
	cheating(false)
{
	initLevel();
}

void GameLogic::resetGame()
{
	level = 1;
	pause = false;
	end = false;
	cheating = false;
	modifs.clear();
	map.map.clear();
	map.collisions.clear();
	timerBlocks.clear();
	collapseBlocks.clear();
	collapsingBlocks.clear();
	player.coinBlocks.clear();
	player.doorOpened = false;
	player.key.collected = false;
	player.coin.collected = false;
	gameAudio.timer.stop();
	gameAudio.music.stop();
	initLevel();
}

void GameLogic::clear()
{
	++level;
	end = true;

	modifs.clear();

	map.map.clear();
	map.collisions.clear();

	timerBlocks.clear();

	collapseBlocks.clear();
	collapsingBlocks.clear();

	gameAudio.timer.stop();

	player.coinBlocks.clear();

	player.doorOpened = false;

	player.key.collected = false;

	player.coin.collected = false;
}

void GameLogic::initLevel() {
	if (level > 2) {
		gameAudio.timer.stop();
		gameAudio.music.stop();
		return;
	}
	timer = timerElLoco = 0;
	godMode = hardcoreMode = interactEvent = timerBlocksDisplayed = elLocoDisplayed = false;
	extractMap();
}

void GameLogic::addModif(int indX, int indY, int value) {
	map.map[indY][indX] = value;
	modifs.push_back({ indX,indY,value,16,16 });
}

int GameLogic::tileType(int tile) const {
	if (tile == EMPTY || tile == COIN || tile == CHAIR_L || tile == CHAIR_R || tile == BOUQUET ||
		tile == DOOR_BC || tile == DOOR_TC || tile == DOOR_BO || tile == DOOR_TO ||
		tile == KEY || tile == CHEST_LC || tile == CHEST_RC || tile == CHEST_LO || tile == CHEST_RO) return -1;
	return tile != LADDER && tile != ROPE && tile != ROPE_TOP && tile != ROPE_END;
}

void GameLogic::interact(int indX, int indY, float deltaTime) {
	if (map.map[indY][indX] == COIN && !player.coin.collected) {
		Coin& coin = player.coin;
		if (player.coin.timer > 0) {
			addModif(coin.y, coin.x, EMPTY);
			coin.timer = 0;
		}
		if (gameAudio.playing) gameAudio.timer.play();
		coin = Coin(indX, indY, 1, 0, true);
		addModif(indX, indY, EMPTY);
		for (auto& block : player.coinBlocks)
		{
			addModif(block.y, block.x, EMPTY);
			map.collisions[block.x][block.y] = -1;
		}
	}
	else if (map.map[indY][indX] == LADDER || map.map[indY + 1][indX] == LADDER || map.map[indY][indX] == ROPE ||
		map.map[indY + 1][indX] == ROPE || map.map[indY][indX] == ROPE_END || map.map[indY + 1][indX] == ROPE_END) {
		float tmp = player.y;
		player.climb(climbing, deltaTime);
		int y = (player.y) / TILE_DIM;
		if (map.collisions[y][indX] > 0) player.y = tmp;
	}
	else if (map.map[indY][indX] == KEY) {
		Key& key = player.key;
		key.collected = true;
		addModif(key.y, key.x, EMPTY);
	}
	else if ((map.map[indY][indX] == CHEST_LC || map.map[indY][indX] == CHEST_RC)) {
		player.key.collected = false;
		addModif(player.key.y, player.key.x, KEY);
		if (map.map[indY][indX] == CHEST_LC) {
			addModif(indX, indY, CHEST_LO);
			if (map.map[indY][indX + 1] == CHEST_RC)
				addModif(indX + 1, indY, CHEST_RO);
		}
		else {
			if (map.map[indY][indX - 1] == CHEST_LC)
				addModif(indX - 1, indY, CHEST_LO);
			addModif(indX, indY, CHEST_RO);
		}
		gameAudio.playSound(gameAudio.chest);
	}
	else if ((map.map[indY][indX] == DOOR_TC || map.map[indY][indX] == DOOR_BC) && player.key.collected) {
		player.doorOpened = true;
		if (map.map[indY][indX] == DOOR_TC) {
			addModif(indX, indY, DOOR_TO);
			addModif(indX, indY + 1, DOOR_BO);
		}
		else {
			addModif(indX, indY, DOOR_BO);
			addModif(indX, indY - 1, DOOR_TO);
		}
		if (player.doorOpened) {
			clear();
			initLevel();
		}
	}
}



void GameLogic::handleEvents(float deltaTime, int keyCode)
{
	if (pause)
		return;

	// Di chuyển trái
	const bool moveLeft =
		sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
	const bool moveRight =
		sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

	if (moveLeft && !moveRight)
	{
		player.move(-1, deltaTime);
	}
	// Di chuyển phải
	else if (moveRight && !moveLeft)
	{
		player.move(1, deltaTime);
	}
	else
	{
		player.vx = 0;
	}

	// Nhảy
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		if (godMode || player.state != Player::JUMP)
		{
			player.vy = JUMP_VELOCITY;
			player.state = Player::JUMP;
			gameAudio.playSound(gameAudio.jumpSoundLeft);
		}
	}

	// Reset vị trí
	if (keyCode == sf::Keyboard::R)
	{
		player.x = player.originX;
		player.y = player.originY;

		player.vx = 0;
		player.vy = 0;
		player.state = Player::IDLE;
	}

	// Bật / tắt âm thanh
	if (keyCode == sf::Keyboard::M)
	{
		gameAudio.changeState();
	}

	// God Mode
	if (keyCode == sf::Keyboard::G)
	{
		if (!cheating)
		{
			cheating = true;
			std::cout << CHEAT_MESSAGE << std::endl;
		}

		godMode = !godMode;
	}

	// Hardcore Mode
	if (keyCode == sf::Keyboard::H)
	{
		hardcoreMode = !hardcoreMode;
	}

	// Leo lên
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		climbing = true;
		interactEvent = true;
	}
	// Leo xuống
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		climbing = false;
		interactEvent = true;
	}

	// Tương tác (mở rương, lấy chìa khóa, mở cửa...)
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		interactEvent = true;
	}
}

void GameLogic::jump(float deltaTime)
{
	if (player.state == Player::JUMP)
	{
		player.vy -= GRAVITY * deltaTime;
		player.y -= player.vy * deltaTime;

		player.sprite.setPosition(player.x, player.y);
	}
}

void GameLogic::initBox(Box& b) const
{
	if (b.flags & 1)
		b.centerX = player.x / TILE_DIM;

	if (b.flags & 2)
		b.centerY = player.y / TILE_DIM;

	if (b.flags & 4)
		b.top = (player.y - PLAYER_DIM_SCALED) / TILE_DIM;

	if (b.flags & 8)
		b.down = (player.y + PLAYER_DIM_SCALED) / TILE_DIM;

	if (b.flags & 16)
		b.left = (player.x - PLAYER_DIM_SCALED) / TILE_DIM;

	if (b.flags & 32)
		b.right = (player.x + PLAYER_DIM_SCALED) / TILE_DIM;
}

void GameLogic::handleCollisions(
	const std::vector<std::vector<sf::Sprite>>& gmap,
	float deltaTime)
{
	Box b;
	b.flags = 63;
	initBox(b);

	sf::FloatRect rect;

	if (interactEvent)
		interact(b.centerX, b.centerY, deltaTime);

	jump(deltaTime);

	//----------------------------------
	// Collision Top / Bottom
	//----------------------------------

	int limY = std::abs((int)player.vy);
	limY = (limY > JUMP_VELOCITY) ? limY / JUMP_VELOCITY : 0;

	for (int i = 0; i <= limY + 1; i++)
	{
		// Top
		if (i &&
			map.collisions[b.top - i][b.centerX] > 0 &&
			player.sprite.getGlobalBounds().intersects(
				gmap[b.top - i][b.centerX].getGlobalBounds(),
				rect))
		{
			if (map.map[b.top - i][b.centerX] != COLLAPSE_BLOCK)
			{
				player.y += rect.height;
				player.vy = 0;

				player.sprite.setPosition(player.x, player.y);

				b.flags = 61;
				initBox(b);

				break;
			}
		}

		// Bottom
		if (map.collisions[b.down + i][b.centerX] > 0 &&
			player.sprite.getGlobalBounds().intersects(
				gmap[b.down + i][b.centerX].getGlobalBounds(),
				rect))
		{
			player.y -= rect.height;

			player.state = (player.vx != 0)
				? Player::RUN
				: Player::IDLE;

			player.spriteRect.left = IDLE_1_OFFSET;

			player.vy = 0;

			player.sprite.setPosition(player.x, player.y);

			b.flags = 61;
			initBox(b);

			break;
		}
	}

	//----------------------------------
	// Collision Left / Right
	//----------------------------------

	int limX = (int)-player.oldX;
	limX = (limX > 16) ? limX / 16 : 0;

	for (int i = 0; i <= limX; i++)
	{
		// Left
		if (map.collisions[b.centerY][b.left - i] > 0 &&
			player.sprite.getGlobalBounds().intersects(
				gmap[b.centerY][b.left - i].getGlobalBounds(),
				rect))
		{
			if (map.map[b.centerY][b.left] != COLLAPSE_BLOCK)
			{
				player.x += rect.width;

				player.sprite.setPosition(player.x, player.y);

				b.flags = 61;
				initBox(b);

				break;
			}
		}

		// Right
		if (map.collisions[b.centerY][b.right + i] > 0 &&
			player.sprite.getGlobalBounds().intersects(
				gmap[b.centerY][b.right + i].getGlobalBounds(),
				rect))
		{
			if (map.map[b.centerY][b.right] != COLLAPSE_BLOCK)
			{
				player.x -= rect.width;

				player.sprite.setPosition(player.x, player.y);

				b.flags = 61;
				initBox(b);

				break;
			}
		}
	}

	//----------------------------------
	// Falling
	//----------------------------------

	if (player.state != Player::JUMP &&
		map.collisions[b.down][b.centerX] == -1)
	{
		player.state = Player::JUMP;
		player.vy = 0;
	}
	else if (!map.collisions[b.down][b.centerX])
	{
		player.vy = 1;
	}

	//----------------------------------
	// Collapse Block
	//----------------------------------

	if (map.map[b.down][b.centerX] == COLLAPSE_BLOCK)
	{
		for (auto& cb : collapseBlocks)
		{
			if (cb.x == b.down &&
				cb.y == b.centerX)
			{
				collapsingBlocks.push_back(cb);
			}
		}
	}
}

void GameLogic::handleCollisions2(
	const std::vector<std::vector<sf::Sprite>>& gmap,
	float deltaTime)
{
	Box b;
	b.flags = 63;
	initBox(b);

	sf::FloatRect rect;

	int i = 1;

	if (map.collisions[b.centerY][b.centerX] > 0 &&
		player.sprite.getGlobalBounds().intersects(
			gmap[b.centerY][b.centerX].getGlobalBounds(),
			rect))
	{
		bool move = false;

		while (true)
		{
			// Trái
			if (b.left - i >= 0 &&
				map.collisions[b.centerY][b.left - i] <= 0)
			{
				player.x -= rect.width + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				move = true;
			}

			// Phải
			else if (b.right + i < map.width &&
				map.collisions[b.centerY][b.right + i] <= 0)
			{
				player.x += rect.width + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				move = true;
			}

			// Trên
			else if (b.top - i >= 0 &&
				map.collisions[b.top - i][b.centerX] <= 0)
			{
				player.y -= rect.height + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				move = true;
			}

			// Dưới
			else if (b.down + i < map.height &&
				map.collisions[b.down + i][b.centerX] <= 0)
			{
				player.y += rect.height + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				move = true;
			}

			// Trái trên
			else if (b.left - i >= 0 &&
				b.top - i >= 0 &&
				map.collisions[b.top - i][b.left - i] <= 0)
			{
				player.x -= rect.width + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				player.y -= rect.height + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				move = true;
			}

			// Phải trên
			else if (b.right + i < map.width &&
				b.top - i >= 0 &&
				map.collisions[b.top - i][b.right + i] <= 0)
			{
				player.x += rect.width + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				player.y -= rect.height + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				move = true;
			}

			// Trái dưới
			else if (b.left - i >= 0 &&
				b.down + i < map.height &&
				map.collisions[b.down + i][b.left - i] <= 0)
			{
				player.x -= rect.width + (i - 1) * TILE_DIM + PLAYER_DIM_SCALED;
				player.y += rect.height + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				move = true;
			}

			// Phải dưới
			else if (b.right + i < map.width &&
				b.down + i < map.height &&
				map.collisions[b.down + i][b.right + i] <= 0)
			{
				player.x += rect.width + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				player.y += rect.height + (i - 1) * TILE_DIM - PLAYER_DIM_SCALED;
				move = true;
			}

			if (move)
				break;

			++i;
		}

		player.sprite.setPosition(player.x, player.y);
	}
}


bool GameLogic::updateCoin(float deltaTime)
{
	Coin& coin = player.coin;

	if (coin.collected)
	{
		coin.timer += deltaTime;

		if (coin.timer > COIN_BLOCK_TIMER)
		{
			addModif(coin.x, coin.y, COIN);

			coin.timer = 0;
			coin.collected = false;

			for (auto& block : player.coinBlocks)
			{
				map.collisions[block.x][block.y] = 1;
				addModif(block.y, block.x, COIN_BLOCK);
			}

			return true;
		}
	}

	return false;
}

bool GameLogic::updateCollapseBlocks(float deltaTime)
{
	for (auto it = collapsingBlocks.begin(); it != collapsingBlocks.end(); ++it)
	{
		it->timer += deltaTime;

		if (it->displayed && it->timer > COLLAPSE_BLOCK_TIMER_1)
		{
			it->displayed = false;
			it->timer = 0;

			addModif(it->y, it->x, EMPTY);
			map.collisions[it->x][it->y] = -1;
		}
		else if (!it->displayed && it->timer > COLLAPSE_BLOCK_TIMER_2)
		{
			it->timer = 0;

			addModif(it->y, it->x, COLLAPSE_BLOCK);
			map.collisions[it->x][it->y] = 1;

			collapsingBlocks.erase(it);
			return true;
		}
	}

	return false;
}

bool GameLogic::updateTimerBlocks(float deltaTime) {
	timer += deltaTime;
	if (timer > TIMER_BLOCK_TIMER) {
		timer = 0;
		size_t size = timerBlocks.size();
		int x, y, blockValue = (timerBlocksDisplayed) ? EMPTY : TIMER_BLOCK;
		timerBlocksDisplayed = !timerBlocksDisplayed;
		for (size_t i = 0;i < size;i++) {
			x = timerBlocks[i].x;
			y = timerBlocks[i].y;
			addModif(y, x, blockValue);
			map.collisions[x][y] = (blockValue == TIMER_BLOCK) ? 1 : -1;
		}
		return true;
	}
	return false;
}

void GameLogic::animateElLoco(float deltaTime)
{
	timerElLoco += deltaTime;

	if (timerElLoco > TIMER_EL_LOCO_2)
	{
		timerElLoco = 0;
		elLocoDisplayed = false;

		player.sprite = player.oldSprite;
	}
	else if (timerElLoco > TIMER_EL_LOCO_1 && !elLocoDisplayed)
	{
		elLocoDisplayed = true;

		player.oldSprite = player.sprite;
		player.sprite = elLoco;
	}
	else if (!elLocoDisplayed)
	{
		player.animate(deltaTime);
	}
}
void GameLogic::update(
	float deltaTime,
	const std::vector<std::vector<sf::Sprite>>& gmap,
	int keyCode)
{
	modifs.clear();

	interactEvent = false;

	handleEvents(deltaTime, keyCode);

	bool reappear = false;


	if (updateCollapseBlocks(deltaTime))
		reappear = true;

	if (updateTimerBlocks(deltaTime))
		reappear = true;

	if (updateCoin(deltaTime))
		reappear = true;

	if (reappear)
		handleCollisions2(gmap, deltaTime);

	handleCollisions(gmap, deltaTime);

	animateElLoco(deltaTime);

	player.sprite.setPosition(player.x, player.y);
}

void GameLogic::extractMap()
{
	ifstream levelMap;
	string filename = PATH_LEVEL;
	char c;
	int tmp;

	filename += to_string(level) + CSV;
	levelMap.open(filename.c_str());
	if (!levelMap.good()) {
		string absPath = "C:/GameProjects/EscapeElLoco/x64/Debug/" + filename;
		levelMap.open(absPath.c_str());
	}
	if (!levelMap.good()) {
		string absPath = "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/" + filename;
		levelMap.open(absPath.c_str());
	}
	if (!levelMap.good()) {
		string absPath = "C:/GameProjects/EscapeElLoco/x64/Debug/" + filename;
		levelMap.open(absPath.c_str());
	}
	if (!levelMap.good()) {
		string absPath = "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/" + filename;
		levelMap.open(absPath.c_str());
	}

	if (levelMap.good())
	{
		int posX1, posX2, posY1, posY2;

		levelMap >> map.height >> c >> map.width >> c;
		levelMap >> posX1 >> c >> posY1 >> c;
		levelMap >> posX2 >> c >> posY2 >> c;

		int sep = map.width >> 1;

		if (level == 2)
			sep++;

		map.map.emplace_back();
		map.collisions.emplace_back();

		for (int i = 0; i < map.height; i++)
		{
			for (int j = 0; j < map.width; j++)
			{
				levelMap >> tmp >> c;

				if (tmp == COIN_BLOCK)
				{
					player.coinBlocks.push_back(CoinBlock(i, j, 1, 0));
				}
				else if (tmp == COIN)
				{
					player.coin = Coin();
				}
				else if (tmp == KEY)
				{
					player.key = Key(i, j, 1, 0);
					tmp = EMPTY;
				}
				else if (tmp == TIMER_BLOCK)
				{
					timerBlocks.push_back(TimerBlock(i, j, 0, true, 1, 0));
					tmp = EMPTY;
				}
				else if (tmp == COLLAPSE_BLOCK)
				{
					collapseBlocks.push_back(CollapseBlock(i, j, 0, 1, 0));
				}

				map.map.back().push_back(tmp);
				map.collisions.back().push_back(tileType(tmp));

			}

			map.map.emplace_back();
			map.collisions.emplace_back();
		}

		levelMap.close();

		player.init(posX1, posY1, "");
	}
	else
	{
		cerr << LOAD_MAP_ERROR;
		exit(1);
	}
}


GameLogic::~GameLogic()
{
}
