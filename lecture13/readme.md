Лекция 13 Контейнеры

контейнеры связаны с аллокаторами

7) Пример нестандартного аллокатора

возможное задание: напишите list с нестандартным аллокатором

StackAllocator<int>
он сразу выделяет дофига памяти (~2 Гб)

1) как должен выглядеть конструктор копирования? 
скопировать все поля - не вариант RunTime Error

Проблема подсчета ссылок
Нужно хранить указатель + счетчик на то, сколько аллокаторов указывают на тоже (умные указатели)

2) через deepcopy()

9 глава контейнеры

какие vector deque list forward_list, set, map, multi_map, multi_set,
unorder_set, unorder_map

они все относятся к 2 группам: sequance containters, accosiated containters

sequance containters: vector, list, deque
accosiated containters: 

Это такие спец объекты которые хранят в себе другие объекты

1) std::vector - это динамический массив

а) добавление/удаление с конца
push_back/pop_back/emplace_back как это работает?

(потренировать для собеса написать push_back на бумажке)

это не перевыделение, а изначально много выделяете, если эл задействовано больше,
чем заранее выделено, то делается relocate и делается x2 и элементы копируются

нашишем ее

template<class T,  class Alloc = std::allocator<T>>
    class vector{
        T* arr;
        size_t sz; // сколько элементов задействовано и реально выделено
        size_t cap;

        public: 
        void push_back(const T& x) {
            if (sz < cap>) {
                arr[sz++] = x;
                return;
            }
        }
    }

это говнокод, надо так 

void push_back(const T& x) {
    if (sz < cap>) {
        new(arr + sz)T(x); // вызов placement new
        return;
    }
}

и эта тоже плохая версия потому что есть аллокатор, контейнер должен все делать через аллокатор!

template<class T,  class Alloc = std::allocator<T>>
    class vector{
        T* arr;
        size_t sz; // сколько элементов задействовано и реально выделено
        size_t cap;
        Alloc alloc;

        public: 
        void push_back(const T& x) {
            if (sz < cap>) {
                alloc.construct(arr + sz, x);
                return;
            }
        }
    }

и это тоже неправильно) потому что в alloc может не быть функции construct, надо 
делать через allocator_traits


void push_back(const T& x) {
    if (sz < cap>) {
        allocator_traits<Alloc>::construct(alloc arr + sz, x);
        return;
    }
}


исправленная версия
template<class T,  class Alloc = std::allocator<T>>
    class vector{
        T* arr;
        size_t sz; // сколько элементов задействовано и реально выделено
        size_t cap;
        Alloc alloc;

        public: 
        void push_back(const T& x) {
            if (sz < cap>) {
                allocator_traits<Alloc>::construct(alloc arr + sz, x);
                return;
            }

        }
    }

введем сокращение using traits = allocator_trais<Alloc>


template<class T,  class Alloc = std::allocator<T>>
    class vector{
        T* arr;
        size_t sz; // сколько элементов задействовано и реально выделено
        size_t cap;
        Alloc alloc;

        public: 
        void push_back(const T& x) {
            if (sz < cap>) {
                allocator_traits<Alloc>::construct(alloc arr + sz, x);
                return;
            }

        }
        T* newarr = traits::allocate(alloc, cap*2); // увеличили емкость
        // теперь нужно перенести все элементы в новый массив
        for (int i = 0; i < cap; ++i) {
            traits::constuct(alloc, newarr + i, arr[i]); // это неправильно, но для 2007 года норм, надо делать через move
        }
        traits::construct(alloc newarr + cap, x); // и это неправильно
        ++sz;
        // как освободить старый массив? delete[]
        // нужно у каждого по отдельности вызвать destroy и ...
        for () {
            destroy(alloc, arr + i);
            traits::deallocate(arr, cap);
        }
        arr = newarr;
        cap <<= 1;
    }

по стандарту требуется, чтобы работало за O(1)

pop_back легко реализовать 
1) 

template<class ... Args>
void emplace_back(const Args& ... args);
и это неправильно

суть в move семантике

что будет если pop_back у пустого массива?) UB

б) [], at

нужен ли const? не нужен
T& operator[] (size_t i) { return arr[i]};

нужна перегрузка для константого вектора
const T& operator[] (size_t i) const { return arr[i]};

at - делает тоже самое, что и [], но если выход за границу, то получаю ошибку std::out_of_range

а нужно ли написать noexcept?
noexcept пишется у тех функций которые подразумеваются что никогда не фейлятся

как у вектора устроен конструктор по умолчанию и констуктор копирования?

в) Конструктор деструктор оператор =

traits::select_on_conterners_copy_construction()

функция swap у вектора
T t = x;
x = y;
y = t;

еще есть функции: size, capacity, shrink_to_fit() она делает размер вектора равным capacity

1 вопрос на экзамене 
vector<int> v(100);
v.shrink_to_fit();
sizeof(v); // чему равно 

sizeof - это то сколько памяти занимает поля v


2 вопрос
int main() {
    std::vector<int> v(100);
    delete[] &(v[0]); // что будет? Runtime error
}