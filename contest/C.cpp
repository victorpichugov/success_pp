#include <iostream>
#include <algorithm>
#include <vector>

// using std::string;

// Желательно получить решение, которое не считывает входной файл целиком в память, т.е., использует лишь константный объем памяти


void uniqe(std::vector<int>& dublicate_str) {
    std::sort(dublicate_str.begin(), dublicate_str.end());
    auto it = std::unique(dublicate_str.begin(), dublicate_str.end());
    dublicate_str.erase(it, dublicate_str.end());
}

int main()
{
    int n;
    std::cin >> n;
    std::vector<int> str(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> str[i];
    }
    uniqe(str);
    for (size_t elem: str) {
        std::cout << elem << std::endl;
    }
    return 0;
}
