


#include <iostream>
#include <vector>


// Через dp

int main() {
    size_t n, m;
    std::cin >> n >> m;

    // 
    std::vector<std::vector<long long>> dp(n, std::vector<long long>(m, 0));

    for (size_t i = 0; i < n; ++i) {dp[i][0] = 1;}
    for (size_t j = 0; j < m; ++j) {dp[0][j] = 1;}

    for (size_t i = 1; i < n; ++i) {
        for (size_t j = 1; j < m; ++j) {
            dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
        }
    }

    std::cout << dp[n - 1][m - 1];

    return 0;
}

