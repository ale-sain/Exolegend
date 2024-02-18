#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <deque>
#include <sstream>
#include <stdexcept>
#include <chrono>

struct Limit {
    float min;
    float max;
};

struct MazeLimit {
    Limit x;
    Limit y;
};

class MazeTime
{
    public:
        MazeLimit     mazeLimit;
        MazeTime() {
            this->mazeLimit.x.min = 0.0;
            this->mazeLimit.x.max = 3.0;
            this->mazeLimit.y.min = 0.0;
            this->mazeLimit.y.max = 3.0;
            this->sizeSquare = 0.25;
        }

        ~MazeTime() {}

        MazeLimit    newLimit() {
            this->mazeLimit.x.min += sizeSquare;
            this->mazeLimit.y.min += sizeSquare;
            this->mazeLimit.x.max -= sizeSquare;
            this->mazeLimit.y.max -= sizeSquare;
            std::cout << "Limit max : " << this->mazeLimit.x.max << " and min : " <<this->mazeLimit.x.min << std::endl;
            return this->mazeLimit;
        }

    private:
        float     sizeSquare;
};