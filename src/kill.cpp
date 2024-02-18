// #include "globals.h"

// void reset() {
//     //initialisation de toutes vos variables avant le début d'un match
//     gladiator->log("Call of reset function");
// }

// void setup() {
//     gladiator = new Gladiator();
//     //enregistrement de la fonction de reset qui s'éxecute à chaque fois avant qu'une partie commence
//     gladiator->game->onReset(&reset);
// }

// float convertAngle (float angle) {
//     return (angle < 0 ? 360 - abs(angle) : angle);
// }


// float distanceWithEnemy(Position enemy, Position myRobot) {
//     float dx = enemy.x - myRobot.x;
//     float dy = enemy.y - myRobot.y;
//     float distance = sqrt(dx * dx + dy * dy);
//     return distance;
// }

void loop() {
    if(gladiator->game->isStarted()) { //tester si un match à déjà commencer
        RobotData myRobot = gladiator->robot->getData();
        RobotList listRobot = gladiator->game->getPlayingRobotsId();
        RobotData enemy = gladiator->game->getOtherRobotData(listRobot.ids[1]);
        gladiator->log("My position :\n\tx-> %.3f\n\ty-> %.3f\n\tangle-> %d\nEnemy position:\n\tx-> %.3f\n\ty-> %.3f\n\tangle-> %d",
            myRobot.position.x, myRobot.position.y, (int)convertAngle(myRobot.position.a * 180 / PI), 
            myRobot.position.x, myRobot.position.y, (int)convertAngle(myRobot.position.a * 180 / PI)
        );
        float distance = distanceWithEnemy(enemy.position, myRobot.position);
        gladiator->log("DISTANCE WITH ENEMY : %.3f", distance);
        if (distance < 1.2)
        {
            gladiator->log("LETS GOOOOOOOOOOO");
            aim(gladiator, {enemy.position.x, enemy.position.y}, false);
        }
        else
            go_to({2.0, 1.0, 0}, myRobot.position, NORMAL);
    }
    delay(100);
}