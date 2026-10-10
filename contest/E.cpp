
#include <iostream>
#include <algorithm>

// using std::string;

bool problem_e(std::string& a, std::string& b) {
    int result = 0;

    for (const char ch : a) {
        result += b.find(ch) != std::string::npos;
    }
    return result == a.size(); 
}

int main()
{
    std::string a, b;
    std::cin >> a >> b;

    std::cout << problem_e(a, b);

    return 0;
}
