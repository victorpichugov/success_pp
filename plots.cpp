#include <iostream>
#include <cmath>
#include <cstdio>

int main() {
    FILE* gp = popen("gnuplot -persist", "w");
    fprintf(gp, "plot sin(x)\n");
    pclose(gp);
    return 0;
}