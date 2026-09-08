#include <iostream>
#include <vector>
#include <chrono>


void processByValue(std::vector<int> data) { // копируется весь вектор!
    for (int x: data) x *= 2;
}

// ХОРОШО: передача по указателю (копируется только адрес - 8 байт)


// vector<int>* data - это указатель на вектор 
void processByPointers(std::vector<int>* data) {
    std::cout << "data = " << data << std::endl; // 0x7ffe420bc6a0
    for (int& x : *data) {
        std::cout << "x = " << x << std::endl;
        //    ↑     ↑
        //    |     └── коллекция (вектор)
        //    └── ссылка на элемент
        // *data - разименовывание
        // for (int& x : *data) - это цикл с диапазоном
        x *= 2;
    }
}

// for (int x : *data) {  // x - это КОПИЯ элемента
//     x *= 2;            // меняем копию, оригинал НЕ меняется!
// }

// for (int& x : *data) { // x - это ССЫЛКА на элемент (псевдоним)
//     x *= 2;            // меняем оригинал в векторе!
// }

// ЕЩЁ ЛУЧШЕ: передача по ссылке (тоже без копирования)
void processByReference(std::vector<int>& data) {
    for (int& x : data) x *= 2;
}


// int main(int argc, char const *argv[])
int main() {
    std::vector<int> bitData(100, 1);

    // По значению - копируется 4 МБ данных!
    processByValue(bitData); // МЕДЛЕННО 

    // По указателю - копируется только 8 байт (адрес)
    processByPointers(&bitData); // БЫСТРО 


    // По ссылке - тоже 8 байт
    processByReference(bitData); // БЫСТРО 
    return 0;
}