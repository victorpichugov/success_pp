#include <iostream>
#include <algorithm>

bool problem_e(std::string& a, std::string& b) {
    if (a.size() != b.size()) return false;
    
    int count[26] = {0};
    for (char ch: a) count[ch - 'a']++;
    for (char ch: b) count[ch - 'a']--;

    for (size_t i = 0; i < 26; ++i)
    {
        if (count[i] != 0) return false;
    }
    

    return true;
}

int main() {
    std::string a, b;
    std::cin >> a >> b;


    std::cout << problem_e(a, b);
    return 0;
}
