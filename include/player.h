#ifndef PLAYER_H
#define PLAYER_H

#include "snake.h"
#include "map.h"
#include <memory>
#include <string>

// 定义玩家控制按键，包括新增的加速键
struct PlayerControls {
    int up, down, left, right, boost;
};

enum class PlayerType { Human, AI };
enum class Team { Red, Blue };

class Player {
public:
    Player(int id, Team team, PlayerType type, const PlayerControls& controls, const Map& gameMap, int startX, int startY, InitialDirection dir);
    ~Player();

    // 接口：获取玩家状态
    int getID() const;
    Team getTeam() const;
    PlayerType getPlayerType() const;
    const PlayerControls& getControls() const;
    Snake& getSnake() const;
    int getScore() const;
    bool isAlive() const;
    const std::string& getName() const;
    bool isBoosting() const;

    // 接口：修改玩家状态
    void addScore(int points);
    void kill();
    void activateBoost();
    void update(); // 用于更新内部状态，如加速回合

private:
    int mID;
    Team mTeam;
    PlayerType mPlayerType;
    PlayerControls mControls;
    std::unique_ptr<Snake> mPtrSnake;
    int mScore;
    bool mIsAlive;
    std::string mName;
    int mBoostTurnsRemaining;
};

#endif // PLAYER_H