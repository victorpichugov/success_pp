#include <algorithm>
#include <iostream>
#include <vector>

using std::string;
using std::cin;
using std::cout; 

int main() {
    string j, s;
    cin >> j >> s;

    int result = 0;
    for (const char ch : s) {
        result += j.find(ch) != string::npos;
    }

    cout << result;
    return 0;
}