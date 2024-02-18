// #include "mazeTime.hpp"
// #include <thread>
// #include <chrono>

// int main() {
//     MazeTime *mazeTime = new MazeTime();
//     auto start = std::chrono::steady_clock::now();
//     int lastPeriod = 0;
//     // Exemple d'utilisation
//     for (int i = 0; i < 100; ++i) {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         auto now = std::chrono::steady_clock::now();
//         std::chrono::duration<double> elapsed_seconds = now - start;

//         // Convertissez elapsed_seconds en un nombre entier de périodes de 20 secondes
//         int periods = static_cast<int>(elapsed_seconds.count() / 5);

//         // Supposons que lastPeriod est une variable membre de votre classe initialisée à -1
//         if (periods > lastPeriod) {
//             // Cela signifie que nous avons atteint une nouvelle période de 20 secondes
//             std::cout << "Reduction applied!" << std::endl;
//             MazeLimit newLimit = mazeTime->newLimit();
//             std::cout << "Limit max : " << newLimit.x.max << " and min : " << newLimit.x.min << std::endl;

//             lastPeriod = periods; // Mettez à jour lastPeriod pour la prochaine vérification
//         }
//         else
//             std::cout << i << std::endl;
//     }
// }