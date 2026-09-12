

#include <iostream>
#include <iomanip>

// TC = O(n) SC = O(1)
double calculatePower(double base, int64_t exponent){
    double ans = 1;

    for (int64_t i = 0; i < exponent; ++i)
        ans *= base;

    return ans;
}

// TC = O(log(n)) SC = O(1)
double fastPower(double base, int64_t exponent){
    if (exponent < 0) {
        base = 1.0 / base;
        exponent = -exponent;
    }
    double result = 1.0;
    while (exponent > 0) {
        if (exponent & 1) result *= base; 
        base *= base;
        exponent >> 1; 
    }

    return result;
}

int main()
{
    double x;
    int64_t n;
    std::cin >> x >> n;
    std::cout << std::fixed << std::setprecision(2) << calculatePower(x, n);
    return 0;
}
