
#include <iostream>
#include <vector>
#include <limits>
#include <cstdint> 

int main()
{
    size_t n;
    std::cin >> n;
    std::vector<int64_t> arr(n);

    for (size_t i = 0; i < n; ++i)
        std::cin >> arr[i];

    // int64_t max = std::numeric_limits<int64_t>::min();
    int64_t max = INT64_MIN;

    for (size_t i = 0; i < n; ++i)
        max = max < arr[i] ? arr[i] : max;

    std::cout << max;
    
    return 0;
}