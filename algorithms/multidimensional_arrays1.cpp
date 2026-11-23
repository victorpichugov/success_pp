


#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    size_t n;
    std::cin >> n;
    std::vector<std::vector<int>> spiral(2*n+1, std::vector<int>(2*n+1, 0));
    int i = n, j = n;
    int dx[] = {0, -1, 0, 1};
    int dy[] = {1, 0, -1, 0};
    size_t direction = 1;

    int step_length = 1; // 1, 1, 2, 2, 3, 3, 4, 4, 5, 5
    int step_done = 0;
    int turns = 0;
    int current_value = 1;

    while (current_value < (2*n +1) * (2*n +1))
    {
        i += dx[direction];
        j += dy[direction];

        spiral[i][j] = current_value;
        current_value++;
        step_done++;

        if (step_done == step_length) { // 
            direction = (direction + 1) % 4; // переключение направления
            step_done = 0; // потому что на каждом новом повороте начинаем считать заново кол-во пройденных шагов?
            turns++; // кол-во поворотов

            if (turns % 2 == 0) { // потому что всегда 2 поворота у цифры
                step_length++; // ув длину шага
            }
        }
    }
    
    for (size_t r = 0; r < 2*n+1; ++r) {
        for (size_t c = 0; c < 2*n+1; ++c) {
            std::cout << std::setw(3) << spiral[r][c] << " ";
        }
        std::cout << "\n";
    }   
    return 0;
}

// int main() {
//     size_t n, m;
//     std::cin >> n >> m;

//     // 
//     std::vector<std::vector<long long>> dp(n, std::vector<long long>(m, 0));

//     for (size_t i = 0; i < n; ++i) {dp[i][0] = 1;}
//     for (size_t j = 0; j < m; ++j) {dp[0][j] = 1;}

//     for (size_t i = 1; i < n; ++i) {
//         for (size_t j = 1; j < m; ++j) {
//             dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
//         }
//     }


//     std::cout << std::setw(0) << std::endl;

//     for (size_t i = 0; i < n; ++i) {
//         for (size_t j = 0; j < m; ++j) {
//             std::cout << std::right << std::setw(6) << dp[i][j];
//         }
//         std::cout << std::setw(0) << std::endl;
//     }

//     return 0;
// }

