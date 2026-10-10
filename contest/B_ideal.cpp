// Требуется найти в бинарном векторе самую длинную последовательность единиц и вывести её длину.

#include <iostream>
#include <vector>
#include <algorithm>


int b_problem(std::vector<int>& bits) {
    int ans = 0;  // Инициализируем нулём!
    int sums = 0;
    
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == 1) {
            sums++;
            if (sums > ans) {
                ans = sums;
            }
        } else {
            sums = 0;
        }
    }
    return ans;
}

std::string test_case() {
    std::vector<int> test1 = {1};
    std::vector<int> test2 = {0};
    std::vector<int> test3 = {1, 0};
    std::vector<int> test4 = {0, 1};
    std::vector<int> test5 = {0, 1, 0};
    std::vector<int> test6 = {1, 0, 0};
    std::vector<int> test7 = {1, 1, 1};
    std::vector<int> test8 = {0, 0, 0};
    std::vector<int> test9 = {0, 0, 0};
    std::vector<int> test10 = {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1};
    std::vector<int> test11 = {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};
    std::vector<int> test12 = {0, 1, 0, 1, 0, 1, 1, 1, 0};

    int fails = 0;

    fails += b_problem(test1) == 1;
    fails += b_problem(test2) == 0;
    fails += b_problem(test3) == 1;
    fails += b_problem(test4) == 1;
    fails += b_problem(test5) == 1;
    fails += b_problem(test6) == 1;
    fails += b_problem(test7) == 3;
    fails += b_problem(test8) == 0;
    fails += b_problem(test9) == 0;
    fails += b_problem(test10) == 4;
    fails += b_problem(test11) == 6;
    fails += b_problem(test12) == 3;

    return fails != 0 ? "All tests have been passed." : 
        "There are errors in the tests.";
}



int main()
{
    int n;
    std::cin >> n; 
    std::vector<int> bits(n);

    for (size_t i = 0; i < n; ++i)
    {
        std::cin >> bits[i]; 
    }

    std::cout << b_problem(bits);

    return 0;
}
