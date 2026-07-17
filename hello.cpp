#include <iostream>
#include <vector>
#include <cmath>

// using namespace std;

// Структуры
struct Point {
    double x;
    double y;
};

struct Segment {
    Point p1;
    Point p2;
};

double length(Segment s) {
    double dx = s.p1.x - s.p2.x;
    double dy = s.p1.y - s.p2.y;

    return sqrt(dx * dx + dy * dy);
}

double length(Segment s) {
    double dx = s->p1.x - s -> p2.x;
    double dy = s->p1.y - s -> p2.y;
    return sqrt(dx * dx + dy * dy);
}

struct IntArray2D
{
    size_t a;
    size_t b;
    int ** data;
};

int main() {
    std::cout << "Привет, Ubuntu! C++ работает122!" << std::endl;

    Point p1 = {0.4, 1.4};
    Point p2 = {1.2, 6.3};
    Segment s = {p1, p2};
    IntArray2D a = {n, m, create_array2d(n, m)}

    return 0;
}
