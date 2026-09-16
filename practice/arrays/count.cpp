
#include <iostream>
#include <vector>

int main()
{
    size_t n;
    std::cin >> n;
    std::vector<int64_t> arr(n);
    int64_t target, count = 0;

    for (size_t i = 0; i < n; ++i)
        std::cin >> arr[i];

    std::cin >> target;

    for (size_t i = 0; i < n; ++i)
        count += arr[i] == target ? 1 : 0;

    std::cout << count;
    
    return 0;
}