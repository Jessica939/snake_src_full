#include "player.h"

Player::Player(int id, Team team, PlayerType type, const PlayerControls& controls, const Map& gameMap, int startX, int startY, InitialDirection dir)
    : mID(id), mTeam(team), mPlayerType(type), mControls(controls), mScore(0), mIsAlive(true), mBoostTurnsRemaining(0)
{
    mName = (mPlayerType == PlayerType::Human) ? "Player " + std::to_string(mID) : "AI " + std::to_string(mID);

    mPtrSnake = std::make_unique<Snake>(gameMap.getWidth(), gameMap.getHeight(), 5);
    mPtrSnake->setMap(const_cast<Map*>(&gameMap));
    mPtrSnake->initializeSnake(startX, startY, dir);
}

Player::~Player() {}

// --- Getter Implementations ---
int Player::getID() const { return mID; }
Team Player::getTeam() const { return mTeam; }
PlayerType Player::getPlayerType() const { return mPlayerType; }
const PlayerControls& Player::getControls() const { return mControls; }
Snake& Player::getSnake() const { return *mPtrSnake; }
int Player::getScore() const { return mScore; }
bool Player::isAlive() const { return mIsAlive; }
const std::string& Player::getName() const { return mName; }
bool Player::isBoosting() const { return mBoostTurnsRemaining > 0; }

// --- Setter/Action Implementations ---
void Player::addScore(int points) { mScore += points; }
void Player::kill() { mIsAlive = false; }
void Player::activateBoost() { mBoostTurnsRemaining = 20; } // 加速持续20帧
void Player::update() { 
    if (mBoostTurnsRemaining > 0) {
        mBoostTurnsRemaining--; 
    }
}