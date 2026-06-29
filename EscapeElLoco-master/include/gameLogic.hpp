#ifndef _GAME_LOGIC_H_
#define _GAME_LOGIC_H_

#include <SFML/Graphics.hpp>
#include "gameAudio.hpp"
#include <vector>
#include <string>
#include "player.hpp"
#include "block.hpp"
#include "object.hpp"
#define POSY 1728
#define PATH_LEVEL "C:/Users/MSI/Downloads/EscapeElLoco-master/EscapeElLoco-master/levels/level"
#define CSV ".csv"
#define LOAD_MAP_ERROR "Error while reading map\n"
#define JUMP_VELOCITY 800
#define GRAVITY 2500
#define PLAYER_DIM_SCALED 32
#define TILE_DIM 64
#define OFFSET 32
#define EMPTY 98
#define COIN 65
#define COIN_BLOCK 25
#define TIMER_BLOCK 12
#define COLLAPSE_BLOCK 66
#define LADDER 56
#define ROPE 15
#define ROPE_END 23
#define ROPE_TOP 7
#define CHEST_LC 30
#define CHEST_RC 31
#define CHEST_LO 92
#define CHEST_RO 93
#define KEY 90
#define DOOR_TC 74
#define DOOR_BC 82
#define DOOR_TO 77
#define DOOR_BO 85
#define TABLE 59
#define CHAIR_L 68
#define CHAIR_R 69
#define BOUQUET 60


#define TIMER_EL_LOCO_1 10.0f
#define TIMER_EL_LOCO_2 10.4f

#define CHEAT_MESSAGE "You cheated not only the game, but yourself.\n\n\
You didn't grow.\nYou didn't improve.\nYou took a shortcut and gained nothing.\n\n\
You experienced a hollow victory.\n\
Nothing was risked and nothing was gained.\n\n\
It's sad that you don't know the difference."

class GameLogic
{
public:

    struct Map
    {
        int height, width;
        std::vector<std::vector<int>> map;
        std::vector<std::vector<int>> collisions;
    };

    struct Box
    {
        unsigned char flags : 6;

        int top;
        int down;
        int left;
        int right;
        int centerX;
        int centerY;
    };

    struct Modif
    {
        int x;
        int y;
        int value;
        int w;
        int h;
    };

    GameAudio& gameAudio;

    int level;

    float timer;
    float timerElLoco;

    bool godMode;
    bool hardcoreMode;
    bool pause;
    bool interactEvent;
    bool timerBlocksDisplayed;
    bool climbing;
    bool end;
    bool elLocoDisplayed;
    bool cheating;

    Map map;

    std::vector<Modif> modifs;

    std::vector<TimerBlock> timerBlocks;

    // Chỉ còn 1 danh sách block
    std::vector<CollapseBlock> collapseBlocks;

    std::vector<CollapseBlock> collapsingBlocks;

    sf::Sprite elLoco;

    // Chỉ còn 1 người chơi
    Player player;
    Coin coin;
    Key key;

    GameLogic(GameAudio& audio);

    void clear();

    void initLevel();

    void extractMap();

    void addModif(int indX, int indY, int value);

    void handleEvents(float deltaTime, int keyCode);

    void jump(float dt);

    void handleCollisions(
        const std::vector<std::vector<sf::Sprite>>& gmap,
        float dt);

    void handleCollisions2(
        const std::vector<std::vector<sf::Sprite>>& gmap,
        float dt);

    void update(
        float deltaTime,
        const std::vector<std::vector<sf::Sprite>>& gmap,
        int keyCode);

    void testCollisions(
        const std::vector<std::vector<sf::Sprite>>& gmap,
        float dt);

    int tileType(int tile) const;

    void interact(
        int indX,
        int indY,
        float deltaTime);
    
    bool updateCoin(float deltaTime);

    bool updateCollapseBlocks(float deltaTime);

    bool updateTimerBlocks(float deltaTime);

    void animateElLoco(float deltaTime);

    void initBox(Box& b) const;

    ~GameLogic();
};

#endif