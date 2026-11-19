

#include <iostream>
#include <vector>
#include <algorithm>
#include <ace/Reactor.h>
#include <ace/Event_Handler.h>
// Лямбда-выражение (или просто лямбда) в C++ — это способ создания анонимной функции прямо в том месте кода, где она используется.

// Главная идея лямбд — избавить программиста от необходимости 
// объявлять именованные функции (или функторы) для коротких, одноразовых операций, 
// особенно при работе со стандартными алгоритмами (STL).


// [список захвата] (параметры) -> тип_возврата {
//     // тело функции
// }


// auto add = [](int a, int b) {return a + b; };

// int main() {
//     int result = add(5, 3);

//     std::cout << result

//     return 0;
// }

int main() {
    std::vector<int> numbers = {4, 1, 7, 2, 9};

    std::sort(numbers.begin(), numbers.end(), [](int a, int b) {return a > b;});

    for (int n: numbers) {std::cout << n << " ";};
    return 0;
}