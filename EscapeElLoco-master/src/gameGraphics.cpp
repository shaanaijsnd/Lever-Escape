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

// ─────────────────────────────────────────────────────────────
//  Helpers nội bộ cho menu
// ─────────────────────────────────────────────────────────────
static void centerText(sf::Text& t, float x, float y)
{
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(x, y);
}

// Vẽ thanh gradient dọc (dùng VertexArray)
static void drawGradientRect(sf::RenderWindow& win,
    float x, float y, float w, float h,
    sf::Color top, sf::Color bot)
{
    sf::VertexArray va(sf::Quads, 4);
    va[0] = sf::Vertex(sf::Vector2f(x,     y),     top);
    va[1] = sf::Vertex(sf::Vector2f(x + w, y),     top);
    va[2] = sf::Vertex(sf::Vector2f(x + w, y + h), bot);
    va[3] = sf::Vertex(sf::Vector2f(x,     y + h), bot);
    win.draw(va);
}

// ─────────────────────────────────────────────────────────────
//  Particle nhỏ lơ lửng (bụi vàng hầm ngục)
// ─────────────────────────────────────────────────────────────
struct MenuParticle {
    sf::Vector2f pos, vel;
    float life, maxLife, size;
    sf::Color col;
};

static std::vector<MenuParticle> spawnParticles(int n)
{
    std::vector<MenuParticle> p;
    p.reserve(n);
    srand(42);
    for (int i = 0; i < n; i++) {
        MenuParticle mp;
        mp.pos  = { (float)(rand() % WINDOW_WIDTH), (float)(rand() % WINDOW_HEIGHT) };
        mp.vel  = { (rand() % 40 - 20) / 60.f, -(rand() % 30 + 10) / 60.f };
        mp.maxLife = mp.life = 180.f + rand() % 240;
        mp.size = 1.f + (rand() % 3);
        int variant = rand() % 3;
        if (variant == 0) mp.col = sf::Color(255, 215,  50, 200);
        else if (variant == 1) mp.col = sf::Color(200, 160, 255, 160);
        else               mp.col = sf::Color(255, 255, 255, 120);
        p.push_back(mp);
    }
    return p;
}

// ─────────────────────────────────────────────────────────────
//  showMenu()
// ─────────────────────────────────────────────────────────────
int GameGraphics::showMenu()
{
    // ── Cấu hình lựa chọn ──────────────────────────────────
    const int OPTION_COUNT = 3;
    std::string labels[OPTION_COUNT]    = { "PLAY",       "CONTROLS",     "QUIT" };
    // 0=Play, 1=Controls (sub-screen), 2=Quit

    int selected = 0;
    bool decided = false;
    bool showControls = false;
    int  result   = 2;

    sf::Clock menuClock;

    // ── Particles ─────────────────────────────────────────
    auto particles = spawnParticles(80);

    // ── Nền tối gradient + lớp map mờ ──────────────────────
    // Tạo render texture cho lớp map mờ
    sf::RenderTexture mapRT;
    mapRT.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    mapRT.clear(sf::Color::Transparent);
    window.setView(window.getDefaultView());
    mapRT.setView(window.getDefaultView());
    for (auto& row : map)
        for (auto& sp : row)
            mapRT.draw(sp);
    mapRT.display();
    sf::Sprite mapSprite(mapRT.getTexture());
    mapSprite.setColor(sf::Color(255, 255, 255, 35)); // sangf rất mờ

    // ── Title chính ────────────────────────────────────────
    // Shadow
    sf::Text titleShadow;
    titleShadow.setFont(font); titleShadow.setCharacterSize(88);
    titleShadow.setFillColor(sf::Color(0, 0, 0, 140));
    titleShadow.setString(TITLE);
    centerText(titleShadow, WINDOW_WIDTH / 2.f + 4.f, 148.f + 4.f);

    sf::Text titleText;
    titleText.setFont(font); titleText.setCharacterSize(88);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setString(TITLE);
    centerText(titleText, WINDOW_WIDTH / 2.f, 148.f);

    // Subtitle italic
    sf::Text subtitleText;
    subtitleText.setFont(font); subtitleText.setCharacterSize(22);
    subtitleText.setFillColor(sf::Color(200, 170, 100, 210));
    subtitleText.setString("~ A Dungeon Escape Adventure ~");
    centerText(subtitleText, WINDOW_WIDTH / 2.f, 215.f);

    // ── Đường kẻ trang trí dưới title ─────────────────────
    sf::RectangleShape divLine(sf::Vector2f(480.f, 2.f));
    divLine.setFillColor(sf::Color(255, 215, 0, 180));
    divLine.setOrigin(240.f, 1.f);
    divLine.setPosition(WINDOW_WIDTH / 2.f, 238.f);

    // ── Khung nền menu ─────────────────────────────────────
    const float PANEL_W = 420.f, PANEL_H = 300.f;
    const float PANEL_X = (WINDOW_WIDTH - PANEL_W) / 2.f;
    const float PANEL_Y = 278.f;

    sf::RectangleShape panelBg(sf::Vector2f(PANEL_W, PANEL_H));
    panelBg.setPosition(PANEL_X, PANEL_Y);
    panelBg.setFillColor(sf::Color(10, 8, 18, 200));

    sf::RectangleShape panelBorder(sf::Vector2f(PANEL_W, PANEL_H));
    panelBorder.setPosition(PANEL_X, PANEL_Y);
    panelBorder.setFillColor(sf::Color::Transparent);
    panelBorder.setOutlineThickness(2.f);
    panelBorder.setOutlineColor(sf::Color(255, 215, 0, 160));

    // Góc trang trí
    auto makeCorner = [](float x, float y, float w, float h) {
        sf::RectangleShape c(sf::Vector2f(w, h));
        c.setPosition(x, y);
        c.setFillColor(sf::Color(255, 215, 0, 220));
        return c;
    };
    float cx = PANEL_X, cy = PANEL_Y, cw = 12.f, ct = 2.f;
    sf::RectangleShape corners[8] = {
        makeCorner(cx,           cy,                    cw, ct),
        makeCorner(cx,           cy,                    ct, cw),
        makeCorner(cx+PANEL_W-cw,cy,                    cw, ct),
        makeCorner(cx+PANEL_W-ct,cy,                    ct, cw),
        makeCorner(cx,           cy+PANEL_H-ct,         cw, ct),
        makeCorner(cx,           cy+PANEL_H-cw,         ct, cw),
        makeCorner(cx+PANEL_W-cw,cy+PANEL_H-ct,         cw, ct),
        makeCorner(cx+PANEL_W-ct,cy+PANEL_H-cw,         ct, cw),
    };

    // ── Texts cho từng option ──────────────────────────────
    const float OPTION_START_Y = PANEL_Y + 50.f;
    const float OPTION_STEP    = 85.f;

    struct MenuBtn {
        sf::Text  text;
        sf::RectangleShape bg;
        sf::RectangleShape accent; // thanh vàng bên trái
    };

    std::vector<MenuBtn> btns(OPTION_COUNT);
    for (int i = 0; i < OPTION_COUNT; i++) {
        float by = OPTION_START_Y + i * OPTION_STEP;

        btns[i].bg.setSize(sf::Vector2f(PANEL_W - 40.f, 60.f));
        btns[i].bg.setPosition(PANEL_X + 20.f, by);

        btns[i].accent.setSize(sf::Vector2f(5.f, 50.f));
        btns[i].accent.setPosition(PANEL_X + 20.f, by + 5.f);
        btns[i].accent.setFillColor(sf::Color(255, 215, 0));

        btns[i].text.setFont(font);
        btns[i].text.setCharacterSize(36);
        btns[i].text.setString(labels[i]);
        centerText(btns[i].text, WINDOW_WIDTH / 2.f, by + 28.f);
    }

    // ── Controls screen ─────────────────────────────────────
    sf::Text ctrlTitle;
    ctrlTitle.setFont(font); ctrlTitle.setCharacterSize(40);
    ctrlTitle.setFillColor(sf::Color(255, 215, 0));
    ctrlTitle.setString("CONTROLS");
    centerText(ctrlTitle, WINDOW_WIDTH / 2.f, 240.f);

    std::string ctrlLines[] = {
        "Arrow LEFT / RIGHT   -   Di chuyen",
        "Arrow UP             -   Leo thang / Tuong tac",
        "Arrow DOWN           -   Xuong thang",
        "SPACE                -   Nhay",
        "E                    -   Mo ruong / Lay chìa khoa",
        "",
        "ESC  -  Quay lai menu",
    };
    std::vector<sf::Text> ctrlTexts;
    for (int i = 0; i < 7; i++) {
        sf::Text t; t.setFont(font); t.setCharacterSize(22);
        t.setFillColor(i == 6 ? sf::Color(180,120,60) : sf::Color(210, 200, 180));
        t.setString(ctrlLines[i]);
        centerText(t, WINDOW_WIDTH / 2.f, 310.f + i * 38.f);
        ctrlTexts.push_back(t);
    }

    // ── Hint phím ────────────────────────────────────────
    sf::Text hintText;
    hintText.setFont(font); hintText.setCharacterSize(17);
    hintText.setFillColor(sf::Color(140, 120, 80, 200));
    hintText.setString("[W/S  hoac  UP/DOWN]  chon     [ENTER/SPACE]  xac nhan     [ESC]  thoat");
    centerText(hintText, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 30.f);

    // ── Version ──────────────────────────────────────────
    sf::Text verText;
    verText.setFont(font); verText.setCharacterSize(15);
    verText.setFillColor(sf::Color(80, 70, 50, 160));
    verText.setString("v1.0");
    verText.setPosition(WINDOW_WIDTH - 50.f, WINDOW_HEIGHT - 28.f);

    // ── Skull deco (dùng ký tự đơn giản) ─────────────────
    sf::Text skullL, skullR;
    skullL.setFont(font); skullL.setCharacterSize(48);
    skullL.setFillColor(sf::Color(255,215,0,80));
    skullL.setString("?"); // placeholder deco
    skullL.setPosition(80.f, 260.f);
    skullR = skullL;
    skullR.setPosition(WINDOW_WIDTH - 120.f, 260.f);

    // ─────────────────────────────────────────────────────
    sf::Event event;

    while (window.isOpen() && !decided)
    {
        float t = menuClock.getElapsedTime().asSeconds();

        // ── Events ──
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            { window.close(); return 2; }

            if (showControls)
            {
                if (event.type == sf::Event::KeyReleased &&
                    (event.key.code == sf::Keyboard::Escape ||
                     event.key.code == sf::Keyboard::Enter))
                    showControls = false;
                continue;
            }

            if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::Up   || event.key.code == sf::Keyboard::W)
                    selected = (selected - 1 + OPTION_COUNT) % OPTION_COUNT;
                else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                    selected = (selected + 1) % OPTION_COUNT;
                else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space)
                {
                    if (selected == 1) showControls = true;
                    else { result = (selected == 0) ? 0 : 2; decided = true; }
                }
                else if (event.key.code == sf::Keyboard::Escape)
                { result = 2; decided = true; }
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                sf::Vector2f mp((float)event.mouseMove.x, (float)event.mouseMove.y);
                for (int i = 0; i < OPTION_COUNT; i++)
                    if (btns[i].bg.getGlobalBounds().contains(mp)) selected = i;
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                sf::Vector2f mp((float)event.mouseButton.x, (float)event.mouseButton.y);
                for (int i = 0; i < OPTION_COUNT; i++)
                    if (btns[i].bg.getGlobalBounds().contains(mp))
                    {
                        selected = i;
                        if (selected == 1) showControls = true;
                        else { result = (selected == 0) ? 0 : 2; decided = true; }
                    }
            }
        }

        // ── Cập nhật particles ──
        for (auto& p : particles) {
            p.pos += p.vel;
            p.life -= 1.f;
            if (p.life <= 0) {
                p.pos  = { (float)(rand() % WINDOW_WIDTH), (float)WINDOW_HEIGHT + 5.f };
                p.life = p.maxLife;
            }
        }

        // ── Tính animation ──
        float pulse     = 0.5f + 0.5f * std::sin(t * 2.5f);          // 0..1
        float titleBob  = std::sin(t * 1.2f) * 5.f;                   // ±5px
        float glowAlpha = (sf::Uint8)(100 + 100 * pulse);

        // Title bobbing
        centerText(titleText,   WINDOW_WIDTH / 2.f, 148.f + titleBob);
        centerText(titleShadow, WINDOW_WIDTH / 2.f + 4.f, 152.f + titleBob + 4.f);

        // Border màu đập theo nhịp
        panelBorder.setOutlineColor(sf::Color(255, 215, 0, (sf::Uint8)(100 + 80 * pulse)));

        // ── Draw ──────────────────────────────────────────
        window.setView(window.getDefaultView());

        // Nền gradient
        drawGradientRect(window, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
            sf::Color(8, 5, 20), sf::Color(22, 14, 42));

        // Map mờ
        window.draw(mapSprite);

        // Particles
        sf::CircleShape dot(2.f);
        for (auto& p : particles) {
            float alpha = (p.life / p.maxLife);
            sf::Color c = p.col;
            c.a = (sf::Uint8)(c.a * alpha);
            dot.setRadius(p.size);
            dot.setFillColor(c);
            dot.setPosition(p.pos);
            window.draw(dot);
        }

        if (!showControls)
        {
            // Title glow (lingering circle phía sau)
            sf::CircleShape glow(220.f);
            glow.setFillColor(sf::Color(80, 50, 0, (sf::Uint8)(18 * pulse)));
            glow.setOrigin(220.f, 220.f);
            glow.setPosition(WINDOW_WIDTH / 2.f, 148.f);
            window.draw(glow);

            window.draw(titleShadow);
            window.draw(titleText);
            window.draw(subtitleText);
            window.draw(divLine);
            window.draw(skullL);
            window.draw(skullR);

            // Panel
            window.draw(panelBg);
            window.draw(panelBorder);
            for (auto& c : corners) window.draw(c);

            // Gradient bên trong panel
            drawGradientRect(window, PANEL_X+2, PANEL_Y+2, PANEL_W-4, PANEL_H-4,
                sf::Color(20, 12, 40, 120), sf::Color(8, 5, 20, 120));

            // Các nút
            for (int i = 0; i < OPTION_COUNT; i++)
            {
                bool sel = (i == selected);
                float scale = sel ? (1.f + 0.03f * pulse) : 1.f;

                // Nền nút
                if (sel) {
                    btns[i].bg.setFillColor(sf::Color(80, 55, 0, 180));
                    btns[i].bg.setOutlineThickness(1.5f);
                    btns[i].bg.setOutlineColor(sf::Color(255,215,0,200));
                } else {
                    btns[i].bg.setFillColor(sf::Color(30, 20, 50, 120));
                    btns[i].bg.setOutlineThickness(1.f);
                    btns[i].bg.setOutlineColor(sf::Color(120,100,60,80));
                }
                window.draw(btns[i].bg);

                // Accent bar bên trái
                if (sel) {
                    btns[i].accent.setFillColor(sf::Color(255, 215, 0, (sf::Uint8)(180 + 60*pulse)));
                    window.draw(btns[i].accent);
                }

                // Text (scale nhẹ khi chọn)
                btns[i].text.setScale(scale, scale);
                centerText(btns[i].text,
                    WINDOW_WIDTH / 2.f,
                    OPTION_START_Y + i * OPTION_STEP + 28.f);
                btns[i].text.setFillColor(sel
                    ? sf::Color(255, 230, 80)
                    : sf::Color(180, 165, 120));
                window.draw(btns[i].text);
            }

            window.draw(hintText);
            window.draw(verText);
        }
        else
        {
            // ── Controls screen ──
            sf::RectangleShape ctrlBg(sf::Vector2f(700.f, 380.f));
            ctrlBg.setFillColor(sf::Color(8, 5, 20, 220));
            ctrlBg.setOutlineThickness(2.f);
            ctrlBg.setOutlineColor(sf::Color(255,215,0,160));
            ctrlBg.setOrigin(350.f, 190.f);
            ctrlBg.setPosition(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f + 30.f);
            window.draw(ctrlBg);

            window.draw(ctrlTitle);
            for (auto& ct : ctrlTexts) window.draw(ct);
        }

        window.display();
    }

    return result;
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
