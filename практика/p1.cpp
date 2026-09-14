

#include <iostream>
#include <iomanip>

// using std::cin;
// using std::cout;


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

    // пример: 3^13 8 + 4 + 1
    // Любое число можно представить как сумму степеней двойки:
    // 13 = 2³ + 2² + 2⁰ 13 = 1101₂
    //                        3210

    // Алгоритм как раз и вычисляет 3¹, 3², 3⁴, 3⁸ 
    // последовательным возведением в квадрат, а потом перемножает только те степени,
    // где в двоичной записи 13 стоит единица.



    while (exponent > 0) {
        if (exponent & 1) result *= base; 
        base *= base;

        std::cout << "exponent1 = " << exponent << std::endl;
        exponent >>= 1; 
        std::cout << "exponent2 = " << exponent << std::endl;

        // exponent >> 1;  именно >>= а не >>
    }

    return result;
}

int main()
{
    double x;
    int64_t n;
    std::cin >> x >> n;
    std::cout << "13 & 1 = " << (13 & 1) << std::endl; 
    std::cout << "27 & 1 = " << (27 & 1) << std::endl; 
    std::cout << "0 & 1 = " << (0 & 1) << std::endl; 
    std::cout << "-1 & 1 = " << (-1 & 1) << std::endl; 

    std::cout << std::fixed << std::setprecision(2) << "calculatePower = " << calculatePower(x, n) << std::endl;
    std::cout << std::fixed << std::setprecision(2) << "fastPower = " << fastPower(x, n) << std::endl;

    return 0;
}
