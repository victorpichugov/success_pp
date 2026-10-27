#include <iostream>
#include <unordered_set>

int main() {
    std::unordered_set<int> numbers = {4, 1, 2, 3, 5, 5};
    for(int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    std::cout << numbers.count(5);

    return 0;
}