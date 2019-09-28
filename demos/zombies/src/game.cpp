#include "game.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

void loadResources(iso::ResourceManager & resManager)
{
    auto & textureWall = resManager.getTexture("res/textures/wall.jpg");
    auto & textureGrass = resManager.getTexture("res/textures/grass.jpg");
    auto & textureCave = resManager.getTexture("res/textures/cave.jpg");
    auto & texturePortal = resManager.getTexture("res/textures/portalRings.png");
    auto & textureZombie = resManager.getTexture("res/textures/zombie.png");
    auto & textureMage = resManager.getTexture("res/textures/magician.png");

    resManager.addAnimation("wall", iso::StaticAnimation(textureWall, iso::math::Vector2i(128, 128)));
    resManager.addAnimation("grass", iso::StaticAnimation(textureGrass, iso::math::Vector2i(128, 128)));
    resManager.addAnimation("cave", iso::StaticAnimation(textureCave, iso::math::Vector2i(128, 128)));
    resManager.addAnimation("portal", iso::Animation(texturePortal, {32, 32}, {0, 0}, {4, 4}, 1, true));

    resManager.addAnimation("zombie-left", iso::Animation(textureZombie, {128, 128}, {64, 64}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-left-up", iso::Animation(textureZombie, {128, 128}, {64, 320}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-up", iso::Animation(textureZombie, {128, 128}, {64, 576}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-right-up", iso::Animation(textureZombie, {128, 128}, {64, 832}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-right", iso::Animation(textureZombie, {128, 128}, {64, 1088}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-right-down", iso::Animation(textureZombie, {128, 128}, {64, 1344}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-down", iso::Animation(textureZombie, {128, 128}, {64, 1600}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("zombie-left-down", iso::Animation(textureZombie, {128, 128}, {64, 1856}, {4, 1}, 1, true, {128, 128}));

    resManager.addAnimation("mage-left", iso::Animation(textureMage, {128, 128}, {64, 64}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("mage-up", iso::Animation(textureMage, {128, 128}, {64, 576}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("mage-right", iso::Animation(textureMage, {128, 128}, {64, 1088}, {4, 1}, 1, true, {128, 128}));
    resManager.addAnimation("mage-down", iso::Animation(textureMage, {128, 128}, {64, 1600}, {4, 1}, 1, true, {128, 128}));
}

Direction getDir(const iso::math::Vector2f & dir)
{
    float angle = std::atan2(dir.y, dir.x) * 180.f / M_PI;
    if (angle >= -22.5f && angle < 22.5f)
        return Direction::Right_Up;
    if (angle >= 22.5f && angle < 67.5f)
        return Direction::Right_Down;
    if (angle >= 67.5f && angle < 112.5f)
        return Direction::Down;
    if (angle >= 112.5f && angle < 157.5f)
        return Direction::Left_Down;
    if (angle >= 157.5f || angle < -157.5f)
        return Direction::Left;
    if (angle <= -112.5f && angle > -157.5f)
        return Direction::Up;
    if (angle <= -67.5f && angle > -112.5f)
        return Direction::Left_Up;
    return Direction::Right;
}

Game::Game()
{
    auto & resManager = iso::ResourceManager::getInstance();
    loadResources(resManager);

    auto portalCb = std::bind(&Game::onPortal, this, std::placeholders::_1);
    player = std::make_shared<PlayerEntity>(portalCb);

    // Render scene, layers
    // Empty -> one layer, default
    engine.reset(new iso::Engine(iso::WindowOptions({spriteSize * width, spriteSize * height}, iso::ResizeStrategy::FIXED_RES_STRETCH, {4, 3}), {"background", "objects"}));

    // Create player sprite and add to engine
    player->setAnimation(resManager.getAnimation("mage-left"));
    player->setPosition({400, 200});
    player->getSprite().setScale(0.25f, 0.25f);
    loadMap();
    engine->addGameObject(player, "objects");
    engine->addRigidBody(player, playerBoundingBox);
    // Equivalent to the previous, no string = top layer
    // engine.addGameObject(player);

    // Can use lambdas instead
    engine->onUpdate += std::bind(&Game::loop, this, std::placeholders::_1);
    engine->onKey += std::bind(&Game::onKey, this, std::placeholders::_1);
}

void Game::loop(float dt)
{
    timeElapsed += dt;

    if (timeElapsed >= respawnTime) {
        timeElapsed -= respawnTime;
        respawn();
    }

    updatePlayer(dt);
    moveZombies(dt);
}

void Game::loadMap()
{
    std::ifstream file("demos/zombies/map.txt");
    if (!file.is_open()) {
        std::cout << "Could not open file" << std::endl;
        return;
    }

    for (unsigned i = 0; i < height; ++i) {
        std::string row;
        file >> row;
        for (unsigned j = 0; j < width; ++j) {
            unsigned tile = row[j] - '0';
            if (Tile(tile) == Tile::Portal) {
                addTile(Tile::Grass, j, i);
            }
            addTile(Tile(tile), j, i);
        }
    }
}

void Game::addTile(Tile tile, int x, int y)
{
    auto & resManager = iso::ResourceManager::getInstance();
    auto obj = std::make_shared<Entity>(Entity::Type::Tile);
    switch (tile) {
    case Tile::Grass:
        obj->setAnimation(resManager.getAnimation("grass"));
        break;
    case Tile::Wall:
        obj->setAnimation(resManager.getAnimation("wall"));
        break;
    case Tile::Portal:
        obj->type = Entity::Type::Portal;
        obj->setAnimation(resManager.getAnimation("portal"));
        if (portalTop == nullptr) {
            portalTop = obj;
        } else {
            portalBottom = obj;
        }
        break;
    case Tile::Cave:
        obj->setAnimation(resManager.getAnimation("cave"));
        break;
    }
    obj->setPosition({float(x * spriteSize), float(y * spriteSize)});
    if (tile != Tile::Portal) {
        obj->getSprite().setScale(0.25f, 0.25f);
    }
    engine->addGameObject(obj, "objects");
    if (tile == Tile::Cave) {
        addRespawnLocation(x, y);
    }
    if (tile != Tile::Grass && tile != Tile::Cave) {
        engine->addRigidBody(obj);
    }
}

void Game::respawn()
{
    for (auto & loc : respawns) {
        createZombie(loc);
    }
}

void Game::run()
{
    engine->run();
}

void Game::addRespawnLocation(int x, int y)
{
    respawns.emplace_back(float(x * spriteSize), float(y * spriteSize));
}

void Game::createZombie(const iso::math::Vector2f & location)
{
    auto & resManager = iso::ResourceManager::getInstance();
    auto zombie = std::make_shared<Entity>(Entity::Type::Zombie);
    zombie->setAnimation(resManager.getAnimation("zombie-left"));
    zombie->setPosition(location);
    zombie->getSprite().setScale(0.25f, 0.25f);
    engine->addGameObject(zombie, "objects");
    zombies.emplace_back(zombie, Direction::Left);
    engine->addRigidBody(zombie, zombieBoundingBox);
}

void Game::moveZombies(float dt)
{
    auto & resManager = iso::ResourceManager::getInstance();
    auto speed = gameSpeed * dt;
    for (auto & zombieInfo : zombies) {
        auto & zombie = zombieInfo.first;
        auto & dir = zombieInfo.second;
        auto playerCenter = player->getPosition() - iso::math::Vector2f(spriteSize / 2, spriteSize / 2);
        auto v = (playerCenter - zombie->getPosition()).normalize();
        zombie->move(v * speed);
        auto newDir = getDir(v);

        if (newDir != dir) {
            dir = newDir;
            if (dir == Direction::Left)
                zombie->setAnimation(resManager.getAnimation("zombie-left"));
            else if (dir == Direction::Up)
                zombie->setAnimation(resManager.getAnimation("zombie-left-up"));
            else if (dir == Direction::Left_Up)
                zombie->setAnimation(resManager.getAnimation("zombie-up"));
            else if (dir == Direction::Right)
                zombie->setAnimation(resManager.getAnimation("zombie-right-up"));
            else if (dir == Direction::Right_Up)
                zombie->setAnimation(resManager.getAnimation("zombie-right"));
            else if (dir == Direction::Right_Down)
                zombie->setAnimation(resManager.getAnimation("zombie-right-down"));
            else if (dir == Direction::Down)
                zombie->setAnimation(resManager.getAnimation("zombie-down"));
            else
                zombie->setAnimation(resManager.getAnimation("zombie-left-down"));
        }
    }
}

std::string dirToString(Direction dir)
{
    switch (dir) {
    case Direction::Left:
        return "Left";
    case Direction::Left_Up:
        return "Left_Up";
    case Direction::Up:
        return "Up";
    case Direction::Right_Up:
        return "Right_Up";
    case Direction::Right:
        return "Right";
    case Direction::Right_Down:
        return "Right_Down";
    case Direction::Down:
        return "Down";
    case Direction::Left_Down:
        return "Left_Down";
    }
}

void Game::updatePlayer(float dt)
{
    auto & resManager = iso::ResourceManager::getInstance();
    auto speed = gameSpeed * dt * 4;
    player->move(playerDir * speed);

    if (playerDir == iso::math::Vector2f(-1, 0))
        player->setAnimation(resManager.getAnimation("mage-left"));
    else if (playerDir == iso::math::Vector2f(0, -1))
        player->setAnimation(resManager.getAnimation("mage-up"));
    else if (playerDir == iso::math::Vector2f(1, 0))
        player->setAnimation(resManager.getAnimation("mage-right"));
    else if (playerDir == iso::math::Vector2f(0, 1))
        player->setAnimation(resManager.getAnimation("mage-down"));

    playerDir = {0, 0};
}

void Game::onKey(iso::KeyEvent event)
{
    switch (event.keyCode) {
    case iso::KeyCode::Left:
        playerDir = {-1, 0};
        break;
    case iso::KeyCode::Up:
        playerDir = {0, -1};
        break;
    case iso::KeyCode::Right:
        playerDir = {1, 0};
        break;
    case iso::KeyCode::Down:
        playerDir = {0, 1};
        break;
    default:
        break;
    }
}

void Game::onPortal(const Entity * portal)
{
    if (portal == portalTop.get()) {
        player->setPosition(portalBottom->getPosition() - iso::math::Vector2f(spriteSize, 0));
    } else {
        player->setPosition(portalTop->getPosition() + iso::math::Vector2f(spriteSize, 0));
    }
}