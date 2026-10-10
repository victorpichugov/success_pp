#include <iostream>
#include <vector>
#include <stdexcept>

// #include <algorithm>

// 0 1
// 1 0
class Solution {
public:
    int maxProfit(std::vector<int>& prices) {
        if (prices.empty()) {
            throw std::invalid_argument("prices can't be empty");
        }
        if (prices.size() < 2) {
            throw std::invalid_argument("size of prices must greater or equal 2");
        }
        int sell = prices[0];
        int buy = prices[1];

        for (size_t i = 1; i < prices.size(); ++i)
        {
            
        }
        
        

        return sell - buy;
    }
};

int main() {
    // std::vector<int> price = {7, 1, 5, 3, 6, 4};
    // Solution s;

    // std::cout << s.maxProfit(price);

    int x = 1;

    int* x1 = 1;
    std::cout << &x; // ссылка в памяти
    std::cout << *x; // ссылка в памяти

    return 0;
}

