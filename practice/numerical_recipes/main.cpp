#include <iostream>
#include <cmath>
#include "calendar.h"
#include "caldat.h"
#include <vector>


double _the_number_of_combinations(const int n, int k) {
    std::vector<double> bnm(n + 1);
    bnm[0] = 1;
    int k0 = 1;

    while (k0 < k) {
        bnm[k0 + 1] = bnm[k0] * (n - k0) / (k0 + 1);
        bnm[n - k0] = bnm[k0];
        k0 += 1; 
    }
    return bnm[k0];
}


int main() {
    std::cout << _the_number_of_combinations(10, 3);
    return 0;
}
