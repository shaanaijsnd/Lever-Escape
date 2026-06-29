#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <SFML/Graphics.hpp>
#include <string>

#include "entity.hpp"
#include "object.hpp"
#include "block.hpp"

#define DEFAULT_SPEED 300.f
#define CLIMB_SPEED   400.f

#define PLAYER_DIM 16

#define RUN_1_OFFSET 414
#define RUN_2_OFFSET 462

#define IDLE_1_OFFSET 4
#define IDLE_2_OFFSET 28

#define JUMP_1_OFFSET 266
#define JUMP_2_OFFSET 220
#define JUMP_3_OFFSET 196

#define OFFSET_Y 4

#define TIMER_IDLE 0.7f
#define TIMER_RUN  0.1f

class Player : public Entity
{
public:

    enum State
    {
        IDLE,
        RUN,
        JUMP
    };

    //------------------------------------
    // Physics
    //------------------------------------

    float vx;
    float vy;

    float oldX;

    float originX;
    float originY;

    //------------------------------------
    // Animation
    //------------------------------------

    float timerIdle;
    float timerRun;

    State state;

    //------------------------------------
    // Game status
    //------------------------------------

    bool chestOpened;
    bool doorOpened;

    //------------------------------------
    // Graphics
    //------------------------------------

    sf::Sprite sprite;
    sf::Sprite oldSprite;

    sf::Texture texture;

    sf::IntRect spriteRect;

    //------------------------------------
    // Objects
    //------------------------------------

    Coin coin;
    Key key;

    std::vector<Coin> coins;
    std::vector<CoinBlock> coinBlocks;

public:

    Player();

    void init(float x,
        float y,
        std::string path);

    void setSprite(sf::Texture& tex);

    void move(int dir,
        float deltaTime);

    void climb(bool climbing,
        float deltaTime);

    void animate(float deltaTime);

    void animateRun(float t1, float t2, State s, int offset1, int offset2);
    
    ~Player();
};

#endif