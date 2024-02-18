#include "gladiator.h"
#include <thread>
#include <chrono>
#include <functional>
#include <cstdio>
#include <atomic>
#include <cmath>
#include "Arduino.h"
#include "user_utils.h"
#include "mazeTime.hpp"
#undef abs

class Vector2 {
public:
    Vector2() : _x(0.), _y(0.) {}
    Vector2(float x, float y) : _x(x), _y(y) {}

    float norm1() const { return std::abs(_x) + std::abs(_y); }
    float norm2() const { return std::sqrt(_x*_x+_y*_y); }
    void normalize() { _x/=norm2(); _y/=norm2(); }
    Vector2 normalized() const { Vector2 out=*this; out.normalize(); return out; }

    Vector2 operator-(const Vector2& other) const { return {_x-other._x, _y-other._y}; }
    Vector2 operator+(const Vector2& other) const { return {_x+other._x, _y+other._y}; }
    Vector2 operator*(float f) const { return {_x*f, _y*f}; }

    bool operator==(const Vector2& other) const { return std::abs(_x-other._x) < 1e-5 && std::abs(_y-other._y)<1e-5; }
    bool operator!=(const Vector2& other) const { return !(*this == other); }

    float x() const { return _x;}
    float y() const { return _y;}

    float dot(const Vector2& other) const { return _x*other._x + _y*other._y; }
    float cross(const Vector2& other) const { return _x*other._y - _y*other._x; }
    float angle(const Vector2& m) const { return std::atan2(cross(m), dot(m)); }
    float angle() const { return std::atan2(_y, _x); }
private:
    float _x, _y;
};

Gladiator* gladiator;
bool allAroundPainted = false;
float lastAngle,nextAngle;
float kw = 1.2;
float kv = 1.f;
float wlimit = 3.f;
float vlimit = 0.6;
float erreurPos = 0.07;

inline float moduloPi(float a) // return angle in [-pi; pi]
{
    return (a < 0.0) ? (std::fmod(a - M_PI, 2*M_PI) + M_PI) : (std::fmod(a + M_PI, 2*M_PI) - M_PI);
}

inline bool aim(Gladiator* gladiator, const Vector2& target, bool showLogs)
{
    constexpr float ANGLE_REACHED_THRESHOLD = 0.1;
    constexpr float POS_REACHED_THRESHOLD = 0.05;

    auto posRaw = gladiator->robot->getData().position;
    Vector2 pos{posRaw.x, posRaw.y};

    Vector2 posError = target - pos;

    float targetAngle = posError.angle();
    float angleError = moduloPi(targetAngle - posRaw.a);

    bool targetReached = false;
    float leftCommand = 0.f;
    float rightCommand = 0.f;

    if (posError.norm2() < POS_REACHED_THRESHOLD) //
    {
        targetReached = true;
    } 
    else if (std::abs(angleError) > ANGLE_REACHED_THRESHOLD)
    {
        float factor = 0.2;
        if (angleError < 0)
            factor = - factor;
        rightCommand = factor;
        leftCommand = -factor;
    }
    else {
        float factor = 0.5;
        rightCommand = factor;//+angleError*0.1  => terme optionel, "pseudo correction angulaire";
        leftCommand = factor;//-angleError*0.1   => terme optionel, "pseudo correction angulaire";
    }

    gladiator->control->setWheelSpeed(WheelAxis::LEFT, leftCommand);
    gladiator->control->setWheelSpeed(WheelAxis::RIGHT, rightCommand);

    if (showLogs || targetReached)
    {
        gladiator->log("ta %f, ca %f, ea %f, tx %f cx %f ex %f ty %f cy %f ey %f", targetAngle, posRaw.a, angleError, target.x(), pos.x(), posError.x(), target.y(), pos.y(), posError.y());
    }

    return targetReached;
}

bool outOfBorder(MazeLimit limits, Position pos) {
    if (pos.x < limits.x.min || pos.x > limits.x.max
        || pos.y < limits.y.min || pos.y > limits.y.max)
        return true;
    return false;
}

int anticipatedUpdate(MazeTime *mazeTime) {
    static bool anticipated = false;
    static int lastPeriod = 0;
    static bool flag = false;
    static std::chrono::time_point<std::chrono::steady_clock> start;

    if (!flag) {
        start = std::chrono::steady_clock::now();
        flag = true;
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = now - start;
    int periods = static_cast<int>(elapsed_seconds.count() / 20);

    int secondsToNextPeriod = 20 - (static_cast<int>(elapsed_seconds.count()) % 20);
    if (secondsToNextPeriod <= 3 && !(anticipated)) {
        mazeTime->newLimit();
        gladiator->log("Anticipating maze reduction, time : %f", elapsed_seconds.count());
        gladiator->log("Limit max : %.2f and min : %.2f", mazeTime->mazeLimit.x.max, mazeTime->mazeLimit.x.min);
        anticipated = true;
    }
    if (periods > lastPeriod) {
        gladiator->log("Reduction !!!! time: %f", elapsed_seconds.count());
        lastPeriod = periods;
        anticipated = false; 
    }
    return secondsToNextPeriod;
}

int around_check()
{
    if (!gladiator->maze->getNearestSquare()->eastSquare && !gladiator->maze->getNearestSquare()->westSquare && !gladiator->maze->getNearestSquare()->southSquare && !gladiator->maze->getNearestSquare()->northSquare)
    {
        gladiator->log("every tile is painted");
        return 1;
    }
    else
    {
        return 0;
    }
}


double reductionAngle(double x)
{
    x = fmod(x + PI, 2 * PI);
    if (x < 0)
        x += 2 * PI;
    return x - PI;
}

void reset() {
    //initialisation de toutes vos variables avant le début d'un match
    gladiator->log("Call of reset function"); // GFA 4.5.1
}

void setup() {
    //instanciation de l'objet gladiator
    gladiator = new Gladiator();
    gladiator->game->onReset(&reset); // GFA 4.4.1
}


void loop() {
    static MazeTime *mazeTime = new MazeTime();
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
    gladiator->log("posX : %f posY:%f maxX:%f maxY:%f minX:%f minY:%f OutOfBorder : %d",gladiator->robot->getData().position.x,gladiator->robot->getData().position.y,mazeTime->mazeLimit.x.max,mazeTime->mazeLimit.y.max,mazeTime->mazeLimit.x.min,mazeTime->mazeLimit.y.min, outOfBorder(mazeTime->mazeLimit,gladiator->robot->getData().position));
        // code de votre stratégie
        //gladiator->log("NEAREST FORWARD :%d",nearestSquare->eastSquare);
        //gladiator->log("degree : %f", gladiator->robot->getData().position.a * 180/3.14f)
        //forwardTilePos->x = (gladiator->maze->getNearestSquare()->i - 2) * 20;
        //forwardTilePos->y = (gladiator->maze->getNearestSquare()->j + 2)* 20;

        static unsigned i = 0;
        static bool chooseNext = true;
        // bool showLogs = (i%50 == 0);
    
        //aim(gladiator, {((gladiator->maze->getNearestSquare()->i + 5) * 0.25 + 0.12),(gladiator->maze->getNearestSquare()->j) * 0.25}, showLogs);

         // code de votre stratégie
        // Position myPosition = gladiator->robot->getData().position;
        RobotList robot_list = gladiator->game->getPlayingRobotsId();
        RobotData Robot2 = gladiator->game->getOtherRobotData(robot_list.ids[1]);
        const MazeSquare* nearestSquare = gladiator->maze->getNearestSquare();
        //gladiator->log("NEAREST FORWARD :%d",nearestSquare->eastSquare);
        static float nextPosX;
        static float nextPosY;
        
        int secondtonextperiod = anticipatedUpdate(mazeTime);
        if (outOfBorder(mazeTime->mazeLimit, gladiator->robot->getData().position)) {
            chooseNext = true;
            aim(gladiator, {1.5, 1.5}, false);
            gladiator->log("GOING TO CENTER");
            //gladiator->log("Limit max : %.2f and min : %.2f", mazeTime->mazeLimit.x.max, mazeTime->mazeLimit.x.min);
           // gladiator->log("currPos x = %.2f, y = %.2f", gladiator->robot->getData().position.x, gladiator->robot->getData().position.y);
        }
        else {
            if (chooseNext)
            {
            if (nearestSquare->eastSquare != 0 && nearestSquare->eastSquare->possession != gladiator->robot->getData().teamId)
            {
                nextPosX = nearestSquare->eastSquare->i * 0.25 + 0.125;
                nextPosY = nearestSquare->eastSquare->j * 0.25 + 0.125;
            }
            else if (nearestSquare->southSquare != 0 && nearestSquare->southSquare->possession != gladiator->robot->getData().teamId)
            {
                nextPosX = nearestSquare->southSquare->i * 0.25 + 0.125;
                nextPosY = nearestSquare->southSquare->j * 0.25 + 0.125;
            }
            else if (nearestSquare->westSquare != 0 && nearestSquare->westSquare->possession != gladiator->robot->getData().teamId)
            {

                nextPosX = nearestSquare->westSquare->i * 0.25 + 0.125;
                nextPosY = nearestSquare->westSquare->j * 0.25 + 0.125;
            }
            else if (nearestSquare->northSquare != 0 && nearestSquare->northSquare->possession != gladiator->robot->getData().teamId)
            {

                nextPosX = nearestSquare->northSquare->i * 0.25 + 0.125;
                nextPosY = nearestSquare->northSquare->j * 0.25 + 0.125;
            }
            chooseNext = false;
            }
            Position next = {nextPosX, nextPosY};
            if (!aim(gladiator, {nextPosX, nextPosY}, false))
            {
                gladiator->log("DONT REACHED");
                gladiator->log("x = %.2f, y = %.2f", next.x, next.y);
            }
            else
                chooseNext = true;
        }
    }
    delay(50);
}
