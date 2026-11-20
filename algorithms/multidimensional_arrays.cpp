


#include <iostream>
#include <vector>


void rotate(
    std::vector<std::vector<int>> &ans, 
    std::vector<std::vector<int>> field,
    size_t n, size_t m
) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            ans[j][n - 1 - i] = field[i][j];
        }
    }
}


int main() {
    size_t n, m;
    std::cin >> n >> m;
    std::vector<std::vector<int>> field(n, std::vector<int>(m, 0));
    std::vector<std::vector<int>> ans(m, std::vector<int>(n, 0));

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            std::cin >> field[i][j];
        }
    }

    rotate(ans, field, n, m);

    std::cout << m << " " << n << std::endl;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            std::cout << ans[i][j] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}

