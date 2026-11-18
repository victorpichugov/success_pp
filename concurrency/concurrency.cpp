
#include <iostream>
#include <vector>
// #include <boost>
#include <thread>

// std::thread 
// std::mutex
// std::async
// boost::

// C++ Thread Library это стандартная библиотека

void hello() {
    std::cout << "Hello world" << std::endl;
}

// // Через лямбда выражения
// std::thread my_thread([](
//     do_something();
//     do_something_else();
// ));

class background_task {
public:
    void operator()() const {
        std::cout << "Поток запущен и работает!\n";
    }
};

int main() {
    // std::thread my_thread(background_task()); // most vexing parse
    // std::thread my_thread((background_task())); // Вариант решения 1 
    std::thread my_thread{background_task()}; // Вариант решения 1 
    my_thread.join();
    return 0;
}




// Вариант 2 - это воспринимается, как объявление функции
// std::thread - это тип возвращаемого значения
// my_thread - это название функции
// background_task() - это объявление параметра этой функции
// получается по итогу так std::thread my_thread(background_task (*)()); 
// int main() {
//     background_task f;
//     std::thread my_thread(background_task());
//     return 0;
// }
