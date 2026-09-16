
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    size_t n;
    std::cin >> n;
    std::vector<int64_t> arr(n);

    for (size_t i = 0; i < n; ++i)
        std::cin >> arr[i];

    // for (size_t i = 0; i < n; ++i)
    //     max = std::max(max, arr[i]);

    int64_t max = *std::max_element(arr.begin(), arr.end());

    std::cout << max;
    
    return 0;
}