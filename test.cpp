#include <iostream>
#include <vector>
#include <chrono>


void processByValue(std::vector<int> data) { // копируется весь вектор!
    for (int& x: data) x *= 2;
}

// ХОРОШО: передача по указателю (копируется только адрес - 8 байт)
void processByPointers(std::vector<int>* data) {
    for (int& x : *data) x *= 2;
}

// ЕЩЁ ЛУЧШЕ: передача по ссылке (тоже без копирования)
void processByReference(std::vector<int>& data) {
    for (int& x : data) x *= 2;
}


// int main(int argc, char const *argv[])
int main() {
    std::vector<int> bitData(1000000, 1);

    // По значению - копируется 4 МБ данных!
    processByValue(bitData); // МЕДЛЕННО 

    // По указателю - копируется только 8 байт (адрес)
    processByPointers(&bitData); // БЫСТРО 


    // По ссылке - тоже 8 байт
    processByReference(bitData); // БЫСТРО 
    return 0;
}