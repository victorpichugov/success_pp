
Лекция 9 Шаблоны

После наследования традиционно

Шаблоны по любому пригодятся

vector<int> это тоже шаблон

Мотивировка swap(int, int)

Я хочу swap(double, double) и swap(bool, bool) неужелия я должен каждый раз так писать?

Хочется объявлять вещи универсальные для всех типов

template (generic в java) - это средство, которое позволяет писать обобщенно

1) Объявление 
Шаблоны бываю у классов и функций

template<список параметров шаблона>

template<typemane T>
    void swap(T& x, T& y) {
        T t = x;
        x = y;
        y = t;
    }

еще можно написать template<class T>

как объявить шаблон класса?

Класс параметризованный типом T
template<typename T>
class C {
    ...
}

int main() {
    int x = 3, y = 2;
    swap(x, y); // если бы было int double

    C<int> c;

}

еще можно было бы указать swap<int>(x, y);

Шаблоны - это еще одно проявление полиморфизма

Виртульальные функции на этапе исполнения после компиляции определяются как будут работать, а шаблоны на этапе компиляции

На собеседовании если спросят что такое полиморфизм в C++
1) перегрузка функции 
2) шаблоны 
3) виртуальные функции

А что тогда наследование и инкапсуляция в C++?
 

2) Специализация шаблонов

template<class T> 
    void f(T& x);

template<> // специализирование шаблонов без параметров
    void f(int x);


Пример f(1) приоритет отдастся 2 методу



template<class T, class Z> 
    void f(T& x, Z& z);

template<>
    void f(int x, int y);

template<class T>
    void f(int x, T& y); // f(5, 0.5);

1) Шаблон всегда старается подобрать точное соответствие
2) Он из всех подходящих версий выбирает наиболее специлизированный


В C++ есть возможность ввести новое название для типа
3) typedef 

typedef (Что) (Как назвать) 
typedef verylongtypename<int, vector<int>> mytype;


пусть
typedef verylongtypename<int, vector<T>> mytype;

хочу mytype<T>


Шаблонный typedef
template<typedef T>
using = verylongtypename<int, vector<T>> mytype;
    mytype<T>

4) Пусть есть шаблонный класс

template<class T>
class C {
    typedef T type;
};

Пусть есть итератор по контейнеру и я не знаю какой тип не через слово auto
Что такое итераторы?

int main() {
    C<T>.:type x; // это ошибка компиляции!
    typename C<T>.:type x; // ok
}

5) revome_const, remove_reference etc
Пример комбинации
void f(T& t) {
    (такого же как T, но если он был const, то снять его, или если он был ссылкой то без нее) x;
}


// Это все на этапе компиляции должно разрешаться

template<class T>
struct remove_const<const T> {
    typedef T type;
}

<T> шаблонный параметр

template<class T>
struct remove_const<T> {
    typedef T type;
}

typename remove_const<T>::type x;

обязательно понять это 

template<class T>
struct remove_reference<T&> {
    typedef T type;
}

<T> шаблонный параметр

template<class T>
struct remove_reference<T> {
    typedef T type;
}

6) 
template<class T>
a) void f(T& x) {}


int main() {
    int x; f(x); // T - int, x - int&
    int& y = x; f(y); // T - int, x - int& 

    const int z = 0; f(z); // T - const int, x - const int&
    const int& t = z; f(t); // T - const int, x - const int&
}

Что подставить под f? 
// Вопрос 1: сохранится ли &?
// Вопрос 2: сохранится ли const?

Ответ: & игнорятся, const сохраняются

Существую универсальные ссылки


б) void f(T x) {}

int main() {
    int x; f(x); // T - int, x - int&
    int& y = x; f(y); // T - int, x - int& 

    const int z = 0; f(z); // T - int, x - int&
    const int& t = z; f(t); // T - int, x - int&
}

Параметры шаблонов не являющимися типами
non-type template parameters


// хочу создать массив константого размера
template<typename T, int n>
class array {

}

array<int, 10> и array<int, 100> - это разные типы!!


array<int, 100> a;
array<int, 200> b;

template<typename T, pointer p> - попробовать!!

еще есть 
template<typename X,  template<typename> T>
void f(const T<X>& t) {

};

3 типа шаблонных параметров:
1) шаблоны являющиеся типами
2) шаблоны являющиеся константы
3) шаблоны являющиеся шаблоны

шаблоны допускают значение по умолчанию
переменное количество шаблонов
template<typename ...>
вот так надо делать переменное количество аргументов