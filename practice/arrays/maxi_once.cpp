
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    std::vector<int64_t> arr;
    int64_t x;

    while (std::cin >> x)
    {
        arr.push_back(x);
    }

    size_t n = arr.size();

    // итераторы на макс и мин 
    auto max_it = std::max_element(arr.begin(), arr.end());
    auto min_it = std::min_element(arr.begin(), arr.end());

    int64_t maxi = *max_it; // значение
    size_t min_index = max_it - arr.begin(); // индекс мин элемента


    for (size_t i = 0; i < n; ++i)
        if (i > min_index)
            std::cout << (i > min_index) ? maxi - arr[i] : arr[i];

    return 0;
}