
#include <iostream>
#include <vector>

// move семантика появилась с 11 и более версий
// она позволяет перемещать ресурсы от одного объекта к другому вместо их копирования
// это решает проблему неэффективного копирования временных объектов


// ЭТО ОЧЕНЬ НЕЭФФЕКТИВНО! 2 РАЗА КОПИРОВАНИЕ
// std::vector<int> createVector() {
//     std::vector<int> v = {1, 2, 3, 4, 5};
//     return v; // копирование всего вектора
// }

// int main() {
//     std::vector<int> my_vec = createVector(); // еще одно копирование 
//     return 0;
// }

// // что такое rvalue/ lvalue ссылки?
// // Move-семантика позволяет забрать ресурсы у временного объекта вместо копирования.
// int main() {
//     int&& rref = 42; // 42 - это временнный объек (rvalue)
//     int x = 10;
//     // int&& rref2 = x; // ссылка rvalue не может быть привязана к lvalueC/C++(1768)
//     int&& rref3 = std::move(x);

//     std::cout << rref;
//     return 0;
// }

// int* - значение по адресу указателя
// int& - адрес в памяти на указатель
// int&& - ссылка

class MyVector {
private:
    int* data;
    size_t size;
public:
    MyVector(size_t s) : size(s), data(new int[s]) {
        std::cout << "Constructor called\n";
    }

    // copy-конструктор (глубокое копирование)
    MyVector(const MyVector& other) : size(other.size), data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        std::cout << "Copy constructor called\n";
    }

    // move-конструктор (перемещение ресурсов)
    MyVector(MyVector&& other) noexcept : size(other.size), data(other.data) {
        other.data = nullptr;
        other.size = 0;
        std::cout << "Move constructor\n";
    }   
    ~MyVector() {
        delete[] data;
    }
};

int main() {
    MyVector v1(10);

    std::cout << "\n--- Копирование v1 в v2 ---\n";
    MyVector v2 = v1; // вызывается конструктор копирования

    std::cout << "\n--- Перемещение v1 в v3 ---\n";
    MyVector v3 = std::move(v1); // вызывается move конструктор

    return 0;
}