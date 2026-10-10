#include <iostream>

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    if (!(std::cin >> n)) return 0;

    if (n == 0) return 0;

    int prev;
    std::cin >> prev;
    std::cout << prev << '\n';

    for (int i = 1; i < n; ++i) {
        int curr;
        std::cin >> curr;
        
        if (curr != prev) {
            std::cout << curr << '\n';
            prev = curr; 
        }
    }

    return 0;
}