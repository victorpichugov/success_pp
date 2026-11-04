# Лекция 14. Value categories и move семантика

> **План лекций после 13-й.** Тема восстановлена по прямым ссылкам из конспектов:
> **лекция 13** — «`traits::construct(alloc, newarr + i, arr[i])` — это неправильно, но для 2007 года
> норм, **надо делать через move**» и «`template<class ... Args> void emplace_back(const Args& ... args);`
> **и это неправильно. суть в move семантике**»; **лекция 5** — «В C++11 вместо правила 3 есть
> **правило 5**»; `vacancy_tasks/README.md` — вопросы ревьюера «**Почему `noexcept` на move?**
> (реаллокация vector → relocation, а не copy)», «**Почему `Add(T)` принимает by value + move,
> а не `const&` или `T&&`?**», «**Что с moved-from объектом и кто отвечает за контракт?**».
> Формат как у лекций 5–12.

---

## 1. Что сломано в `push_back` / `emplace_back` из 13-й лекции

На 13-й лекции мы написали `vector` руками через `allocator_traits` и остановились на двух
«неправильно». Обе дыры чинятся **move-семантикой** — это и есть тема лекции.

### 1а. Реаллокация зовёт конструктор копирования

Из конспекта 13-й:

```cpp
T* newarr = traits::allocate(alloc, cap * 2);
for (int i = 0; i < cap; ++i)
    traits::construct(alloc, newarr + i, arr[i]);   // «неправильно, но для 2007 года норм»
```

Почему неправильно: `arr[i]` — это **lvalue** (у него есть адрес, живёт дольше выражения),
поэтому `construct` выводит overload resolution на **конструктор копирования**. Итог: каждое
удвоение ёмкости **перекапывает** всё содержимое. Для `std::string` — `new` + `memcpy` на каждый
элемент, для тяжёлого объекта из `vacancy_tasks/task4_batch` — 24 МБ на кадр.

Чинится одной вставкой — **`std::move`**:

```cpp
traits::construct(alloc, newarr + i, std::move(arr[i]));   // переезд указателями, O(1) на элемент
```

### 1б. `emplace_back(const Args& ... args)`

Из конспекта 13-й:

```cpp
template<class ... Args>
void emplace_back(const Args& ... args);   // «и это неправильно»
```

Две болезни:

1. **лишние копии** — аргументы принимаются по `const&`, и внутри `construct` они уезжают
   как lvalue → конструкторы копирования;
2. **rvalue не двигаются** — `v.emplace_back(std::string("abc"))` попадает в `const std::string&`
   и строка **копируется**; идеал — построить объект **из rvalue** (или вообще из `(10, 'x')`
   прямо на месте, без промежуточного объекта).

Пакет параметров шаблона (10-я лекция) в связке с `T&&` и `std::forward` лечит обе.

### 1в. Финальный правильный код (продолжение кода 13-й лекции)

```cpp
template<class T, class Alloc = std::allocator<T>>
class MyVector {
    using traits = std::allocator_traits<Alloc>;   // «введем сокращение» из 13-й
    T* arr_ = nullptr;
    size_t sz_ = 0, cap_ = 0;
    Alloc alloc_;

    void grow() {
        size_t newcap = cap_ ? cap_ * 2 : 1;
        T* newarr = traits::allocate(alloc_, newcap);
        size_t built = 0;
        try {
            for (; built < sz_; ++built)
                traits::construct(alloc_, newarr + built, std::move(arr_[built]));  // MOVE
        } catch (...) {
            for (size_t i = 0; i < built; ++i) traits::destroy(alloc_, newarr + i);
            traits::deallocate(alloc_, newarr, newcap);
            throw;                                  // старое содержимое не тронуто
        }
        for (size_t i = 0; i < sz_; ++i) traits::destroy(alloc_, arr_ + i);
        traits::deallocate(alloc_, arr_, cap_);     // порядок «деструкторы → память» из 12-й
        arr_ = newarr;
        cap_ = newcap;
    }

public:
    ~MyVector() {
        for (size_t i = 0; i < sz_; ++i) traits::destroy(alloc_, arr_ + i);
        traits::deallocate(alloc_, arr_, cap_);
    }
    void push_back(const T& x) {                    // lvalue-перегрузка: копия
        if (sz_ == cap_) grow();
        traits::construct(alloc_, arr_ + sz_, x);
        ++sz_;
    }
    void push_back(T&& x) {                         // rvalue-перегрузка: move
        if (sz_ == cap_) grow();
        traits::construct(alloc_, arr_ + sz_, std::move(x));
        ++sz_;
    }
    template<class... Args>
    void emplace_back(Args&&... args) {             // БЕЗ const!
        if (sz_ == cap_) grow();
        traits::construct(alloc_, arr_ + sz_, std::forward<Args>(args)...);
        ++sz_;
    }
    size_t size() const { return sz_; }
    T& operator[](size_t i) { return arr_[i]; }
};
```

```cpp
MyVector<std::string> v;
std::string s = "hello";
v.push_back(s);              // копия: s жив и цел
v.push_back(std::move(s));   // move: s стал пустым
v.emplace_back(10, 'x');     // строка (10, 'x') построена прямо в буфере, промежуточных объектов нет
```

Проверено сборкой: печатает `3 10 src=0` (три элемента, третья строка длины 10, `s.size() == 0`).

> **Вопрос на понимание:** почему `grow()` обёрнут в `try`? Потому что `construct` может бросить,
> и мы обязаны откатить **построенное** и **память** (12-я лекция: «память → конструкторы →
> деструкторы → память»), иначе утечка. Старый буфер при этом не тронут — **сильная гарантия**
> (см. лекцию 11).

---

## 2. Value categories: как это проверять на практике

### 2а. Три практических теста

Категорию выражения определяют три вопроса.

| Тест | lvalue | rvalue |
|---|---|---|
| Можно ли взять `&`? | **да** (`&a`) | **нет** (`&(a + b)`) |
| Можно ли присвоить `x = ...`? | **да** | **нет** (у встроенных) |
| Живёт ли после конца выражения? | **да** | **нет** — умрёт в конце полного выражения |

```cpp
int a = 1, b = 2;
int* p = &a;          // OK
int sum = a + b;      // sum — lvalue, (a + b) — rvalue
// int* q = &(a + b); // не скомпилируется: lvalue required as unary '&' operand

struct C { int x; };
C make() { return C{}; }
// C* p = &make();    // не скомпилируется: taking address of rvalue
```

Проверено: обе строки с `//` g++ отвергает.

**Rvalue — это то, что умрёт в конце выражения** (временный объект). **Lvalue — то, у чего есть
имя/адрес и что живёт дольше выражения.**

### 2б. Дерево категорий

```
              expression
              /        \
        glvalue        rvalue
        /    \         /     \
   lvalue   xvalue  prvalue
                    (имени нет, «сгорит» в конце выражения)
```

- **glvalue** (generalized) — есть адрес; «можно взять `&`»;
- **rvalue** — «можно двигать»; сгорит в конце выражения;
- **lvalue** — glvalue + имя (переменная, результат `*p`, `arr[i]`, `f()`, где возврат `T&`);
- **prvalue** (pure rvalue) — rvalue без адреса: `5`, `C{}`, `a + b`, возврат `T` по значению;
- **xvalue** (e**x**piring value) — и glvalue, и rvalue: есть адрес, но жить будет недолго,
  его **разрешено двигать**. Это `std::move(x)`, `T&&`-параметр внутри функции,
  возврат `T&&`.

> Отсюда формулировка на собесе: **всё, что можно двигать — rvalue; всё, у чего можно взять `&` — glvalue;
> `xvalue` — единственная категория, попавшая в обе ветки.**

**Временный объект** (`std::string("abc")` — prvalue) живёт одно полное выражение. Именно
поэтому на него нельзя повесить `std::string&` (см. 3б) и нельзя
`std::string_view sv = std::string("abc");` — `view` останется висячим.

---

## 3. Правила категорий

### 3а. `&` у rvalue взять нельзя

```cpp
int a = 1, b = 2;
int* q = &(a + b);   // не скомпилируется: lvalue required as unary '&' operand
```

Логика: `&` обещает адрес живущего объекта; временный объект живёт до `;` — указатель был бы
гарантированно висячим.

### 3б. `const T&` продлевает жизнь временному

```cpp
const std::string& s = std::string("abc");   // КОМПИЛИРУЕТСЯ
std::cout << s.size();                        // 3: объект жив до конца области видимости s
```

Почему: связывание **константной** ссылки с prvalue продлевает жизнь временного объекта
до жизни ссылки (ссылка становится владельцем). Нестандартное (`&`) значение:

```cpp
std::string& s = std::string("abc");   // не скомпилируется:
                                       // cannot bind non-const lvalue reference to an rvalue
const char* p = std::string("abc").c_str();   // UB: строка умрёт в конце строки, p висячая
```

### 3в. Присваивание rvalue: встроенным нельзя, своим классам можно

С 6-й лекции у нас висела проблема:

```cpp
3 + 4 = 5;        // ошибка: левый операнд = обязан быть lvalue
c1 + c2 = 5;      // для наших классов КОМПИЛИРУЕТСЯ!
```

Причина: `c1 + c2` — временный объект типа `C`, а у `C` есть `operator=`, который принимает
`*this` без ограничений. Мы закрывали это **`const`-возвратом** (6-я лекция):

```cpp
const C operator+(const C& x, const C& y) { ... }
// c1 + c2 = 5;  → не скомпилируется: passing 'const C' as 'this' argument discards qualifiers
```

(проверено: с не-`const` возвратом компилируется, с `const` — ошибка компиляции).

### 3г. Квалификаторы `&` / `&&` (C++11) — «правильный» ответ на ту же проблему

```cpp
struct C {
    void f() &;      // вызывается только на lvalue
    void g() &&;     // вызывается только на rvalue (забрать ресурсы из временного)
    C& operator+(const C&) &;
};

C c;
c.f();              // OK
std::move(c).g();   // OK
C{}.g();            // OK
// c.g();           // не скомпилируется: passing 'C' as 'this' argument discards qualifiers
```

Практический смысл: `g() &&` — способ сказать «этот объект умирает, забирай его буфер».
То же применимо к `operator=`:

```cpp
C& operator=(const C&) &;   // присваивать можно только в живущий объект
// make() = 5;              // не скомпилируется
```

---

## 4. `T&&`, `std::move`, `std::forward`

### 4а. Rvalue-ссылка

`T&&` — **вторая** ссылка: привязывается к rvalue и даёт право его двигать.

```cpp
int&& r = 5;        // OK
int a = 5;
int&& r2 = a;       // не скомпилируется: cannot bind rvalue reference of type 'int&&' to lvalue
```

### 4б. `std::move` — это каст, он НИЧЕГО не двигает

```cpp
template<class T>
T&& my_move(T& x) {
    return static_cast<T&&>(x);   // вот и вся реализация
}
```

`std::move(x)` делает ровно одну вещь: превращает lvalue в **xvalue** (rvalue-значение), чтобы
перегрузка выбрала конструктор/присваивание от `T&&`. **Никаких указателей он не переставляет.**
Перемещает — **твой** конструктор `T(T&&)`. Отсюда все «каверзы» с `std::move`.

```cpp
String a("hello");
String b(my_move(a));       // вызван String(String&&): a.data_ == nullptr, a.size_ == 0
std::cout << b.size_ << " " << a.size_;   // 5 0
```

### 4в. `std::forward` сохраняет категорию

```cpp
template<class T>
void store(T&& x) {
    Tracked t(std::forward<T>(x));   // lvalue → lvalue (копия), rvalue → rvalue (move)
}
```

Счётчиками проверено: `store(a)` → **1 конструктор копирования**, `store(Tracked("b"))` →
**1 move-конструктор**. `std::forward` живёт **только в шаблоне** с выводом типа: у него явный
шаблонный параметр `std::forward<T>(x)`, и вне шаблона он не выводится
(`no matching function for call to 'forward(int&)'`).

### 4г. Reference collapsing

Правило схлопывания ссылок (4 комбинации):

| Выражение | Результат |
|---|---|
| `X& &` | `X&` |
| `X& &&` | **`X&`** |
| `X&& &` | `X&` |
| `X&& &&` | `X&&` |

Почему `X& &&` → `X&`: ссылка на ссылку нелегальна, а шаблон обязан уметь подставить `T = X&`
в `T&&`; стандарт решил, что **левая ссылка побеждает** (если любая из двух ссылок lvalue —
результат lvalue). Проверено `static_assert`'ами:

```cpp
template<class T> struct AddLvalueRef { using type = T&; };
template<class T> struct AddRvalueRef { using type = T&&; };

static_assert(std::is_same_v<typename AddRvalueRef<int&>::type, int&>,  "X& && -> X&");
static_assert(std::is_same_v<typename AddRvalueRef<int>::type,  int&&>, "X&& && -> X&&");
```

(Механизм тот же, что на 9-й лекции: `remove_reference<T&>` через частичную специализацию +
`std::is_same` из `<type_traits>`.)

### 4д. Почему `T&&` в шаблоне = универсальная ссылка

```cpp
template<class T> void f(T&& x);
f(a);          // a: int lvalue → T = int&  → параметр int& && → схлопывается в int&
f(5);          // 5: int prvalue → T = int   → параметр int&&
```

Ключевое: **вывод типа** + `T&&`. У нешаблонной функции никакой универсальности нет:
`void g(int&& x)` принимает **только** rvalue.

> **Вопрос на собесе:** чем `T&&` в `template<class T> void f(T&&)` отличается от `void f(int&&)`?
> Первый принимает **всё** (и lvalue, и rvalue) и сохраняет категорию для `std::forward`;
> второй — только rvalue, внутри него `x` — **lvalue** (у него есть имя!).

---

## 5. Move-конструктор и move-присваивание руками

### 5а. На классе `String` (урезанная версия из 5-й лекции)

```cpp
struct String {
    char* data_ = nullptr;
    size_t size_ = 0;

    explicit String(const char* s);
    String(const String& o);                      // копия: своё выделение + memcpy (5-я лекция)

    String(String&& o) noexcept                   // MOVE: украли указатели
        : data_(o.data_), size_(o.size_) {
        o.data_ = nullptr;                        // источник ОБЯЗАТЕЛЬНО обнулить
        o.size_ = 0;                              // иначе двойное delete[] в двух деструкторах
    }

    String& operator=(String&& o) noexcept {
        if (this != &o) {                         // 1) self-move guard (как &s == this из 5-й)
            delete[] data_;                       // 2) освободить СВОЁ
            data_ = o.data_;                      // 3) украсть ЧУЖОЕ
            size_ = o.size_;
            o.data_ = nullptr;                    // 4) обнулить источник
            o.size_ = 0;
        }
        return *this;
    }

    String& operator=(const String& o) {
        if (this != &o) { String tmp(o); swap(tmp); }   // copy-and-swap (сильная гарантия, 11-я)
        return *this;
    }

    void swap(String& o) noexcept {
        std::swap(data_, o.data_);
        std::swap(size_, o.size_);
    }

    ~String() { delete[] data_; }
};
```

Порядок move-присваивания — **своё → чужое → обнулить**. Наоборот (сначала украсть, потом
освободить) — при `x = std::move(x)` освобождаем украденное у самого себя.

### 5б. На `vector` (из 13-й лекции)

```cpp
MyVector(MyVector&& o) noexcept
    : arr_(o.arr_), sz_(o.sz_), cap_(o.cap_), alloc_(std::move(o.alloc_)) {
    o.arr_ = nullptr;      // три указателя + обнуление источника: O(1), 0 аллокаций
    o.sz_ = 0;
    o.cap_ = 0;
}
```

### 5в. Что двигает move контейнера и умных указателей

- `std::vector` — переезжают 3 указателя (начало/конец/ёмкость), **ни одного** конструктора
  элемента (проверено: `b = std::move(a)` на `vector<P>` из 1000 элементов даёт
  `elem_copies=0 elem_moves=0`);
- `std::string` — SSO: короткая строка **копируется** из стека, длинная — переезжает буфер;
- `std::unique_ptr` — указатель переезжает, источник becomes `nullptr`;
- `std::shared_ptr` — переезжает указатель + **общий** контрольный блок (упоминание
  «проблемы подсчёта ссылок» из 13-й), `use_count()` **не меняется**;
- встроенные типы (`int`, `double`, указатели, `std::array<int,3>`) — `std::move` = копия,
  двигать нечего.

### 5г. `swap` — самый дешёвый move

На 13-й `swap` вектора был написан «`T t = x; x = y; y = t;`» — это конструктор + два присваивания.
С move это 3 обмена указателями:

```cpp
void swap(MyVector& o) noexcept {
    std::swap(arr_, o.arr_); std::swap(sz_, o.sz_); std::swap(cap_, o.cap_);
}
```

> `swap` — canonical «noexcept-операция, точка невозврата» из copy-and-swap (см. лекцию 11).

---

## 6. `noexcept` на move: почему это критично

### 6а. Правило

`vector` при реаллокации переносит элементы **move-ом только если move не бросает**, то есть
выполняется `std::is_nothrow_move_constructible_v<T>` (в libstdc++ —
`__uninitialized_move_if_noexcept_a`). Иначе он **копирует**: только копия позволяет вернуть
прежнее состояние при броске → **сильная гарантия** (см. лекцию 11).

Отсюда ответ ревьюеру: «**реаллокация vector — relocation, а не copy**» работает **только**
при `noexcept` на move. Без него — тихая деградация в `O(n)` аллокаций + копирование буферов.

### 6б. Эксперимент (счётчики, 16 `push_back`, ёмкости 1→2→4→8, переносим 1+2+4+8 = 15)

```cpp
template<bool NoEx>
struct Item {
    std::string s;
    Item() = default;
    explicit Item(std::string x) : s(std::move(x)) {}
    Item(const Item& o) : s(o.s) { ++copies; }
    Item(Item&& o) noexcept(NoEx) : s(std::move(o.s)) { ++moves; }   // единственное отличие
    Item& operator=(const Item&) = default;
    Item& operator=(Item&&) = default;
};
```

Результат прогона:

| move | copies | moves |
|---|---|---|
| `noexcept` | **0** | 31 |
| без `noexcept` | **15** | 16 |

Снятие `noexcept` = **+15 копирований** буфера на тех же 16 `push_back`.

### 6в. Инструменты

```cpp
std::is_nothrow_move_constructible_v<T>   // трейт: «двигается и не бросает» (9-я лекция, <type_traits>)
std::is_nothrow_move_assignable_v<T>
std::move_if_noexcept(x)                  // «двигать, только если noexcept», иначе копия
void f(T x) noexcept(noexcept(g(x)))      // идиома условного noexcept из 11-й
```

У стандартных типов move `noexcept`: `std::string`, `std::vector`, `std::unique_ptr`,
`std::shared_ptr`, `int` — все дают `true`.

### 6г. Терминация

Если move **не** `noexcept`, а конструктор копирования бросает — попааем в худший сценарий:
восстановить старое содержимое уже нельзя (элементы частично перенесены) →
**`std::terminate`** (проверено: программа падает с `Killed by SIGABRT`, `what(): boom`).
С `noexcept`-move и бросающим копированием та же программа **ловит** исключение и вектор
остаётся валидным (базовая гарантия, 11-я лекция).

> `noexcept` на move — не «оптимизация», а **часть контракта контейнера**: `push_back` обещает
> O(1) амортизированно и сильную гарантию, и без `noexcept` он своё обещание выполнить не может.

---

## 7. Moved-from контракт

### 7а. Формулировка

После `std::move(x)` объект `x` находится в состоянии **«валидном, но неопределённом»**.

Гарантировано:

- **деструктор** отработает корректно (двойного освобождения нет — источник обнулён);
- можно **присвоить** новое (`x = ...`);
- можно **свайпнуть** (`x.swap(y)`);
- для контейнеров — **`size()`/`empty()`** (и `clear()`, `push_back`): стандарт говорит, что
  «валидное состояние, размер unspecified»; на практике libstdc++ даёт пустой контейнер
  (проверено: `s.size() == 0` после `std::string("abc")`-move, `points_.empty() == true` в
  `task4_batch`).

НЕЛЬЗЯ: читать значение, сравнивать с прежним, считать «гарантированно пусто» для не-контейнеров,
переиспользовать объект без явного восстановления.

### 7б. `x = std::move(x)`

Стандарт на самоперемещении **ничего не обещает** (кроме «объект остаётся валидным» для
библиотечных типов). Практика:

- свои классы — **обязан** спасать guard `if (this != &o)` (как `&s == this` из 5-й лекции);
- без guard последствия: `n = 0`/потеря данных, а при чтении из уже освобождённого буфера —
  use-after-free (UB; в моей сборке ASan промолчал, потому что источник и приёмник один объект
  и указатель обнулился, — UB от этого не перестаёт быть UB);
- `std::string`/`std::vector` на самоприсваивании не падают (проверено: `t = std::move(t)` →
  `t.size() == 0`).

### 7в. Кто отвечает за контракт (ответ ревьюеру)

**Автор класса.** Он пишет `T(T&&)` и `operator=(T&&)` и обязан оставить источник в состоянии,
в котором деструктор и все методы, объявленные в контракте класса, работают. Стандарт задаёт
только минимум («валидное»); **конкретное** состояние (пусто? нулевой указатель? пересоздан?)
определяет документация класса и его тесты. В `task4_batch` контракт зафиксирован тестом:
после move `Empty() == true`, `Frame() == nullptr`, `use_count()` не изменился.

### 7г. Swap-идиома

```cpp
void Stack::push(const T& x) {
    std::vector<T> copy = data_;   // если бросит — data_ не тронут
    copy.push_back(x);
    data_.swap(copy);              // noexcept — точка невозврата
}
```

`swap` — единственная операция, которая обязана быть `noexcept`, поэтому она и ставится
**последней**. (copy-and-swap, см. лекцию 11.)

---

## 8. Правило пяти и правила генерации

На 5-й лекции мы записали: «**В C++11 вместо правила 3 есть правило 5**» — вот оно.

| # | Операция | Сигнатура |
|---|---|---|
| 1 | деструктор | `~T()` |
| 2 | конструктор копирования | `T(const T&)` |
| 3 | присваивание копированием | `T& operator=(const T&)` |
| 4 | **конструктор перемещения** | `T(T&&) noexcept` |
| 5 | **присваивание перемещением** | `T& operator=(T&&) noexcept` |

**Правило пяти:** нужен один из пяти — нужны все пять; не нужен ни один — не пишем ни одного.

### 8а. Когда компилятор сам генерирует move, а когда нет

| Что объявлено пользователем | Move генерируется? |
|---|---|
| ничего | **да**, `noexcept` (тривиально) |
| **копирование** (`T(const T&)` или `operator=(const T&)`), в т. ч. `= delete` | **НЕТ** (move = удалён) |
| свой **деструктор** в C++11 | **НЕТ** → объект «двигается» копией |
| свой деструктор в C++17 | **да**, если копирование не объявлено |
| всё пять через `= default` | да |

Проверено компилятором:

```cpp
struct NoDtor  { int x = 0; };                       // is_nothrow_move_constructible = true
struct WithDtor{ int x = 0; ~WithDtor() {} };        // C++17: true
struct WithCopy{ int x = 0; WithCopy(const WithCopy&) {} };  // is_move_constructible = FALSE
```

Практический якорь: **`copy = delete` (тяжёлый объект) + свой деструктор ⇒ `vector<T>` не
компилируется.** Прогон `struct Heavy` с `Heavy(const Heavy&) = delete` и без move-операций:
`push_back(std::move(h))` → **`use of deleted function 'Heavy::Heavy(const Heavy&)'`** +
`static assertion failed: result type must be constructible from value type of input range`.
Это ровно «красное» состояние задачи `task4_batch`.

### 8б. `= default` / `= delete`

```cpp
struct T {
    T() = default;
    T(const T&) = delete;                 // копирование запрещено (тяжёлый объект)
    T& operator=(const T&) = delete;
    T(T&&) noexcept = default;            // вернуть генерацию, которую убила строка выше
    T& operator=(T&&) noexcept = default;
    ~T() = default;                       // «деструктор нужен, но пустой» → не убивает move
};
```

Лечение «деструктор убил move»: **продефолтить все пять**. Одна строчка `~T() {}` в классе —
это не «мелочь», а **отключение move** в C++11 и потеря `noexcept`-движения в общем случае.

### 8в. Правило нуля

Лучший вариант: **не писать ни одной из пяти** — все поля являются RAII-владельцами
(`std::vector`, `std::string`, `std::unique_ptr`, `std::shared_ptr`), и компилятор генерирует
корректные операции сам. Это мост к **15-й лекции** (умные указатели): там «правило нуля»
становится рабочим стилем, а `~T()` появляется только у настоящего владельца ресурса.

---

## 9. Применение

### 9а. `Add(T x) { v_ = std::move(x); }` — почему by value + move

Три варианта приёма параметра и их цена (подсчитано счётчиками):

| Сигнатура | `Add(t)` (lvalue) | `Add(std::move(t))` (rvalue) | Комментарий |
|---|---|---|---|
| `Add(const T& x)` | **1 копия** | **1 копия** | rvalue не двигается; `x` неизменяем, владения нет |
| `Add(T&& x)` | **не компилируется** | 1 move | lvalue-вариант потерян, caller обязан писать `std::move` |
| `Add(T x)` + `std::move` | 1 копия + **1 move** | **1 move** | принимает оба; максимум один move; `x` — локальный владелец |

Ответ ревьюеру: **by value** — потому что функция **забирает владение** (значение уходит в
`trajs_`). `const&` не отдаёт владение и всегда стоит копию. `T&&` ломает вызов
`Add(t)` для lvalue (проверено: **не компилируется**, `use of deleted function
'Heavy::Heavy(const Heavy&)'`).

```cpp
void Add(SimpleTrajectory traj) { trajs_.push_back(std::move(traj)); }
```

### 9б. `push_back(std::move(obj))`

```cpp
std::vector<Big> v;
Big b("данные");
v.push_back(b);              // копия: b цел
v.push_back(std::move(b));   // move: b пустой (moved-from контракт)
```

После `std::move(b)` использовать `b` как «значение» нельзя; можно присвоить новое или свайпнуть.

### 9в. Возврат по значению, RVO/NRVO

```cpp
S plain() { S local; return local; }             // NRVO: 0 копий, 0 move
S moved() { S local; return std::move(local); }  // НЕЛЬЗЯ: move запрещает NRVO
```

Замерено: `plain()` → `copies=0 moves=0`; `moved()` → `moves=1` **плюс** предупреждение
компилятора `moving a local object in a return statement prevents copy elision
[-Wpessimizing-move]`. Правило: **пиши `return local;`**, `std::move` в `return` не нужен —
для локального объекта в `return` стандарт сам делает *implicit move*, а `std::move` только
мешает оптимизации.

### 9г. `std::move` в списке инициализации полей (5-я лекция)

```cpp
SimpleTrajectory(std::shared_ptr<const ReferenceFrame> frame)
    : frame_(std::move(frame)) {}     // поле инициализируетсяmove'ом параметра
```

Список инициализации — единственное место, где поле инициализируется (а не присваивается),
поэтому `std::move` здесь даёт **один** move-конструктор вместо пары «конструктор + присваивание».

### 9д. Вернуть by value, а не `const&` (вопрос ревьюера)

```cpp
std::vector<SimpleTrajectory> ExtractAll_move()  { return std::move(trajs_); }
std::vector<SimpleTrajectory> ExtractAll_plain() { return trajs_; }
```

Почему **by value**: функция **отдаёт владение** целиком; вернув `const&`, мы оставили бы
два независимых «хозяина» данных (consumer не знает, жив ли батч, батч не знает, что данные
разобрали). По замеру на gcc 11: `ExtractAll_plain()` оставляет поле прежним
(`batch.size() == 1000`), а `return std::move(trajs_)` делает его пустым (`== 0`).
Если контракт «батч пустеет» — **явный `std::move` обязателен**; если контракт «отдать копию
дешевле» — пиши `return trajs_;` и он всё равно сдвигает (implicit move для rvalue-выражения
из `std::move`-поля здесь не срабатывает, поэтому явный move и нужен).

---

## 10. Каверзы

1. **`std::move(const T&)` не двигает.** `std::move` от `const`-объекта даёт `const T&&`,
   move-конструктор `T(T&&)` под него не подходит → вызывается **конструктор копирования**.
   Проверено: `sink(std::move(const_s))` компилируется и делает **копию**.
2. **`std::move` контейнера двигает буфер, а не элементы.** `b = std::move(a)` — ноль
   конструкторов элементов (замерено: `elem_copies=0 elem_moves=0` на 1000 элементов).
   И наоборот: `std::move` элемента внутри реаллокации — это **один** move на элемент.
3. **`std::move` встроенного типа = копия.** `int`, `double`, сырой указатель,
   `std::array<int,3>` — после `std::move` значения **остаются прежними**
   (замерено: `e[0] == 1` после `std::move(arr)`).
4. **`std::forward` только в шаблонном `T&&`.** Вне шаблона он не выводит тип
   (`no matching function for call to 'forward(int&)'`).
5. **`&&`-квалифицированный метод** не вызывается на lvalue:
   `c.g()` при `void g() &&` → ошибка компиляции.
6. **`std::move` в логировании.** `LOG(std::move(req)); req.Log();` — после лога объект пуст.
   Любая строка, где `std::move` стоит **до** использования, — дефект; правило: `std::move`
   всегда **последним** использованием объекта.
7. **`x = std::move(x)`** — свои классы спасает только guard `this != &o` (см. 7б).
8. **Один `~T() {}`** в классе с `copy = delete` лишает объект move → он **неперемещаем**
   и не живёт в `std::vector`.

---

## 11. Практика

### 11а. Основная работа: `vacancy_tasks/task4_batch`

Формат работы — «**тест ДО фикса** → красное → фикс → зелёное» (как в `practice/`).

```
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
```

Текущее («красное») состояние `batch.h` — **ошибка компиляции**, и это правильно:

```
/usr/include/c++/11/bits/stl_uninitialized.h:138:72: error: static assertion failed:
    result type must be constructible from value type of input range
/usr/include/c++/11/ext/new_allocator.h:162:11: error: use of deleted function
    'SimpleTrajectory::SimpleTrajectory(const SimpleTrajectory&)'
```

Потому что `SimpleTrajectory` объявляет копирование `= delete`, а move-операций нет —
правило генерации (см. 8а) их **не создало**. Падают ровно три места из `batch.h`:
`ExtractAll()` (возврат `trajs_` по значению), `Add()` (`push_back(std::move(traj))`)
и будущий `vector<TrajectoryBatch>`.

Что делать (ТЗ из `vacancy_tasks/task4_batch/README.md`):

1. дописать в `SimpleTrajectory` move-конструктор и move-присваивание, **оба `noexcept`**,
   исходник после move — `Empty() == true`;
2. `TrajectoryBatch`: move-конструктор/присваивание (`noexcept`), `Add(SimpleTrajectory)`
   by value + `std::move`, `ExtractAll()` → `return std::move(trajs_)`;
3. тесты в `main.cpp` (счётчик `failures`, `return failures`): `static_assert`'ы
   (`is_nothrow_move_constructible_v`, `!is_copy_constructible_v`), move-конструктор
   (`a.Empty()`, `a.Frame() == nullptr`, `frame.use_count()` не изменился),
   move-присваивание + self-assignment `d = std::move(d)`, `Add`×3 / `ExtractAll` / повторный
   `ExtractAll`, `vector<TrajectoryBatch>` с `push_back(std::move(batch))` × 1000,
   перф-тест: перенос батча 10⁶ точек — бюджет 10 мс.

Зелёный прогон (эталон в `reference/`): `ALL TESTS PASSED: 0 failure(s)`, перенос батча
— **микросекунды** (3 указателя вместо 24 МБ).

### 11б. Мини-задачи (свои, считаются счётчиками)

1. **Свой `vector::push_back` с move.** Написать `push_back(const T&)`, `push_back(T&&)` и
   `emplace_back(Args&&...)` поверх `allocator_traits` (код из 1в), проверить, что
   `emplace_back(10, 'x')` не создаёт промежуточных `std::string`.
2. **Посчитать копии.** Завести глобальные счётчики `ctor/copy/move/dtor` и снять замеры:
   4 `emplace_back` без `reserve` → **7 конструкторов** (4 + 3 move) и 7 деструкторов;
   с `reserve(4)` → **4 и 4**; 2 `push_back(lvalue)` без `reserve` → 4 конструктора
   (1 + 2 копии + 1 move) и 4 деструктора.
3. **`noexcept`-эксперимент.** Тот же класс с `Item(Item&&) noexcept(NoEx)`; 16 `push_back`
   (см. 6б): снять `noexcept` → **копий с 0 до 15**. Прогнать под `-fsanitize=address`,
   чтобы убедиться, что утечек нет — проблема чисто скоростная.

---

## 12. За рамками лекции (что доучить)

- **rvalue `this` (`this T&&`, C++23)** — квалифицированные методы доводятся до «забрать
  объект целиком»: `void push(this MyVector&& self, T x)`; позволяет корректно двигать `*this`
  из `&&`-контекста (наши `&&`-квалифицированные методы из 3г — их C++11-предок);
- **`std::forward_like<T>(x)` (C++23)** — «forward по типу другого выражения», нужен в
  конструкторах/универсальных обёртках, где тип параметра не выводится;
- **`std::move_only_function` (C++23)** — `std::function`, принимающий только move-only
  замыкания (замыкание с `unique_ptr`-захватом);
- **концепт `std::move_constructible` / `std::movable` (C++20)** — формализация «умест в
  контейнер»; `static_assert`'ы из 11а превращаются в ограничения шаблона;
- **P2546 (implicit move для конструкторов, C++23)** — «не пиши `return std::move(x)`»
  распространяется и на инициализацию базовых классов;
- **`std::optional` / `std::variant`** — move-only варианты, «владение без `new`» (мост к 15-й);
- **`= delete`/`= default` + `virtual`**: `= default` обязателен для виртуального деструктора,
  иначе 8-я лекция (утечка потомка) складывается с правилом пяти;
- литература: **`literature/Эффективный и современный C++.pdf`** (Мейерс, items про
  rvalue-ссылки, `std::move`/`std::forward`, «pass by value»), **`literature/Effective C++.pdf`**
  (правила 3/4/13/14 — база, к которой добавилось правило пяти),
  **`literature/C++17 STL. Стандартная библиотека шаблонов 2018.pdf`** (контейнеры и move),
  **`literature/N4860.pdf`** ([class.copy.ctor], [container.requirements.general] — цитаты
  про moved-from и `is_nothrow_move_constructible`).

> Задача после лекции: довести `vacancy_tasks/task4_batch` до зелёного в формате
> **тест ДО фикса**: сначала написать тесты (a–f из README задачи, включая перф-бюджет 10 мс),
> увидеть красное (ошибку компиляции), затем починить `batch.h`
> (move-конструктор/присваивание `noexcept`, `Add` by value + `std::move`,
> `return std::move(trajs_)`) и сдать «готово» + полный вывод `./t`.
> Команда: `g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t`, плюс прогон с
> `-fsanitize=address` и с счётчиками копий (до/после `noexcept`).

---

## Короткая шпаргалка

| Тема | Главное |
|---|---|
| **Две дыры из 13-й** | `traits::construct(alloc, newarr + i, arr[i])` → конструктор **копирования**; `emplace_back(const Args&...)` → лишние копии, rvalue не двигаются |
| **Как чинить** | `std::move(arr[i])` при реаллокации; `template<class... Args> emplace_back(Args&&...)` + `std::forward` |
| **Rvalue** | То, что **умрёт в конце выражения**; `&` взять нельзя; `5`, `a + b`, `C{}`, возврат по значению |
| **Lvalue** | Есть имя/адрес, живёт дольше выражения; `a`, `arr[i]`, `*p`, возврат `T&` |
| **Дерево** | glvalue = lvalue + **xvalue**; rvalue = **prvalue** + **xvalue**; xvalue — в обеих ветках |
| **`&` от rvalue** | **не компилируется**: `lvalue required as unary '&' operand` |
| **`const T& s = T()`** | **продлевает жизнь** временному до конца области видимости `s` |
| **`T& s = T()`** | **не компилируется**: cannot bind non-const lvalue reference to an rvalue |
| **`c1 + c2 = 5`** | Для своих классов компилируется (6-я лекция); лечится `const`-возвратом, а в C++11 — квалификатором `&` на `operator=` |
| **`void f() & / void g() &&`** | Квалификатор `this`: `g() &&` зовётся только на rvalue; `c.g()` — ошибка компиляции |
| **`T&&`** | Rvalue-ссылка; `int&& r = a;` — **не компилируется** |
| **`std::move`** | = `static_cast<T&&>`; **НИЧЕГО не двигает**; двигает твой `T(T&&)` |
| **`std::forward`** | Сохраняет категорию; **только** в шаблонном `T&&`; вне шаблона не компилируется |
| **Reference collapsing** | `X& &&` → `X&` (levая ссылка побеждает); `X&& &&` → `X&&` |
| **Универсальная ссылка** | `template<class T> f(T&& x)`: `f(a)` → `int&`, `f(5)` → `int&&`; внутри `x` — **lvalue** |
| **Move-конструктор** | Украли указатели + **обнулили источник**; `noexcept`; 0 аллокаций |
| **Move-присваивание** | Порядок: **guard `this != &o` → освободить своё → украсть чужое → обнулить** |
| **`move vector`** | Переезжают **3 указателя**, конструкторов элементов **ноль** |
| **`move` встроенных** | = **копия**; `std::array<int,3>` после move сохраняет значения |
| **`noexcept` на move** | `vector` двигает только при `is_nothrow_move_constructible_v<T>`, иначе **копирует** (сильная гарантия, 11-я) |
| **Замер** | 16 `push_back`: `noexcept` → 0 копий; без `noexcept` → **15 копий** |
| **`copy = delete` + move без `noexcept`** | `vector<T>` **не компилируется** (static assert «must be constructible») |
| **Moved-from** | «Валидное, но неопределённое»: деструктор, присваивание, swap, `size()`/`empty()` |
| **`x = std::move(x)`** | Стандарт ничего не обещает; спасает только guard `this != &o` |
| **Кто отвечает за контракт** | **Автор класса**; минимум задаёт стандарт, конкретику — тесты (в `task4_batch`: `Empty() == true`) |
| **Правило пяти** | dtor, copy ctor, copy `=`, **move ctor**, **move `=`** — все или никто (обещано на 5-й) |
| **`copy` объявлено (даже `= delete`)** | Move **не генерируется** |
| **Свой `~T()`** | C++11: убивает move (двигается **копией**); C++17: генерирует, если копирование не объявлено → **продефолтить все пять** |
| **Правило нуля** | Не писать ни одной из пяти: все поля — RAII-владельцы (→ лекция 15) |
| **`Add(T x)` by value + move** | Принимает lvalue и rvalue, максимум **один move**; `const&` = копия всегда; `T&&` = lvalue не пройдёт |
| **Возврат по значению** | `return local;` — **RVO/NRVO, 0 копий**; `return std::move(local)` — **запрещает** NRVO (`-Wpessimizing-move`) |
| **`std::move` в init list** | `C(std::shared_ptr<F> f) : frame_(std::move(f)) {}` — 1 move вместо ctor + `=` |
| **by value vs `const&` (кэш)** | Отдаём владение → by value; `const&` = два владельца и висячие ссылки |
| **`std::move(const T&)`** | Даёт `const T&&` → вызывается **копирование** |
| **Логирование** | `std::move` — **последнее** использование объекта; иначе объект пуст в следующей строке |
| **`swap`** | Самый дешёвый move; `noexcept`; точка невозврата copy-and-swap (11-я) |
| **Экзамен** | 4 `emplace_back` без `reserve` → **7/7**; с `reserve(4)` → **4/4**; 16 `push_back(T())` → **47/47** (move) и 47/47 (copy: 15 копий вместо 15 move) |
