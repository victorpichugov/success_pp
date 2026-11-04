# Лекция 15. RAII и умные указатели: `unique_ptr`, `shared_ptr`, `weak_ptr`

> Аккуратный конспект лекции Ильи Мещерина по C++. Здесь закрываем пять «висяков» из прошлых лекций.
>
> > «идея: а что если написать спец класс который в конструктор принимает указатель, а
> > в деструкторе делает delete от указателя - **это умные указатели!!! (smart pointers)**
> > **их цель решать проблемы утечки памяти**» (лекция 11, п. 6а)
>
> > «**что такое сборка мусора - в c++ такого нет!** а в java есть» (лекция 11)
>
> > «`*` - это разименование (**умные указатели**)» и повисший с той лекции вопрос
> > «**Что такое умный указатель?**» (лекция 6, унарные `*`, `&`, `->`)
>
> > «**Проблема подсчета ссылок. Нужно хранить указатель + счетчик** на то, сколько
> > аллокаторов указывают на тоже (**умные указатели**)»; «**как должен выглядеть конструктор
> > копирования?**» → «через `deepcopy()`»?» (лекция 13)
>
> > «`std::allocate_shared`/`std::make_unique` — та же тема выделения памяти «под ключ»»,
> > а `<memory>` — «умные указатели, `allocator`» (лекция 12)
>
> > «**Почему `frame` — `shared_ptr`, а кэш — `unique_ptr`?**» (вопрос ревьюера, `vacancy_tasks/README.md`)
>
> Черновик лекции — в `readme.md` рядом.

---

## 1. RAII — ответ на утечку из 11-й лекции

```cpp
struct Ugly {
    int* p;
    Ugly()  { p = new int; throw 1; }
    ~Ugly() { delete p; }      // НИКОГДА не вызовется
};
int main() { try { Ugly u; } catch (...) {} }      // утечка p
```

**Почему утекает:** бросок из конструктора → объект **не достроен** → **деструктор этого
объекта не вызывается** (свои деструкторы вызовут только базы и поля, чьи конструкторы
успели отработать). Всё, что накоплено «руками» в конструкторе, теряется. Отсюда же и
стремность `p = new ...; throw ...; delete p;` — **освобождение не должно зависеть от
потока управления** (лекция 11).

### 1а. Механизм спасения

Компилятор гарантирует: **при любом выходе из области видимости** вызываются деструкторы
всех локальных объектов — и при `}`, и при `return`/`break`, и при `throw`
(**stack unwinding**, лекция 11). Значит, если ресурс лежит **в поле объекта**, он
освобождается **гарантированно**.

> **RAII (Resource Acquisition Is Initialization):** ресурсом владеет объект, ресурс
> acquired в конструкторе, **освобождается в деструкторе**. Время жизни ресурса = время
> жизни объекта. Никаких ручных `delete` «по потоку управления».

### 1б. «Ресурс» — это не только память

| Ресурс | acquire в ctor | release в dtor |
|---|---|---|
| `FILE*` | `fopen` | `fclose` |
| сокет / соединение с IP-сетью (пример из лекции 11) | `connect` | `disconnect` |
| мьютекс | `m.lock()` | `m.unlock()` |
| `int*` из `new` | `new` | `delete` |
| дескриптор SDK | `Open…` | `CloseHandle` / `Release` |
| слот в пуле, «начало/конец» в логе | регистрация | снятие |

> **RAII для мьютекса** — это `std::lock_guard` / `std::scoped_lock` (C++17): ctor берёт
> блокировку, dtor снимает. Полный разбор — на плановой **лекции 21** («Потоки, гонки,
> мьютексы»); здесь строим инструмент, которым она пользуется.

```cpp
class MyLock {                     // своя копия lock_guard — тот же RAII
    std::mutex& m_;
public:
    explicit MyLock(std::mutex& m) : m_(m) { m_.lock(); }
    ~MyLock() { m_.unlock(); }
    MyLock(const MyLock&) = delete;            // второй владелец блокировки = баг
    MyLock& operator=(const MyLock&) = delete;
};
```

### 1в. Класс-владелец: минимальная рамка

```cpp
class File {
    FILE* f_;
public:
    explicit File(const char* path) : f_(std::fopen(path, "w")) {
        if (!f_) throw std::runtime_error("no file");   // бросок из ctor: f_ ещё nullptr → утечки нет
    }
    ~File() { if (f_) std::fclose(f_); }                // noexcept по умолчанию
    File(const File&) = delete;                         // владение не делится
    File& operator=(const File&) = delete;
    void write(const char* s) { std::fputs(s, f_); }
};
```

> **Деструктор обязан быть `noexcept`** — бросок из деструктора при размотке уводит
> программу в `std::terminate` (правило Мейерса, лекция 11): внутри
> `try { close(); } catch (...) { /* лог / игнор */ }`.
> Копирование владельца **запрещаем** (`= delete`, лекция 14 — rule of 5), иначе два
> объекта освободят один ресурс.

### 1г. Сборки мусора в C++ нет

> «Сборка мусора в C++ есть?» — **Нет** (лекция 11: «в c++ такого нет! а в java есть»).
> За ресурсы отвечают **деструкторы** (RAII). GC ищет недостижимые объекты в рантайме; в
> C++ нет ни рантайм-метаданных, ни «остановки мира», ни перемещающего сборщика. Плата —
> дисциплина: **владелец ресурса назван в типе**.

**Ответ на вопрос, повисший с 6-й лекции** («Что такое умный указатель?»):

> **Умный указатель** — **объект, который хранит указатель на объект и управляет его
> жизненным циклом** (обычно освобождает память в деструкторе) — формулировка 6-й лекции.
> Он держит ресурс в себе → реализует RAII; наружу — унарный `*` (разименование) и `->`
> (лекция 6: «**возвращаемое значение должно само поддерживать операцию `->`**»).

---

## 2. `auto_ptr` — исторический урок (зачем нужен «честный» указатель)

```cpp
std::auto_ptr<C> a(new C);
std::auto_ptr<C> b = a;      // «копирование»… и ОБНУЛЕНИЕ ИСТОЧНИКА: a == nullptr
```

**Конструктор копирования с побочным эффектом.** `b` владеет, `a` пуст: объект потерял
value-семантику («копия» ≠ «такой же объект»).

```cpp
std::auto_ptr<C> f(const std::auto_ptr<C>& p) { return p; }   // НЕ компилируется:
// binding reference of type ‘std::auto_ptr<C>&’ to ‘const std::auto_ptr<C>’ discards qualifiers

std::vector<std::auto_ptr<C>> v;
v.push_back(a);     // НЕ компилируется: push_back принимает const&, а «копия» хочет не-const источник
```

`auto_ptr` **удалён в C++17**. В gcc 11 он ещё виден, но только как deprecated-объявление:
`'std::auto_ptr' is deprecated: use 'std::unique_ptr' instead`.

> **Вывод, на котором держится вся лекция:** умный указатель обязан быть **честным** —
> либо копия есть настоящая копия (тогда `shared_ptr`), либо копии нет вообще (тогда
> `unique_ptr`, move-only). «Копия, которая крадёт» — не годится.

---

## 3. `unique_ptr` — эксклюзивное владение

```cpp
template<class T, class D = default_delete<T>>
class unique_ptr { T* p_; /* + deleter, если он stateful: он часть типа! */ };
```

**Нет** конструктора копирования и копирующего `=` → **move-only**;
`unique_ptr<C> b = a;` — **не скомпилируется** (проверено).
`sizeof(unique_ptr<T>) == 8`, если deleter **stateless**.

### 3а. `make_unique` (C++14) против `new`

```cpp
auto p = std::make_unique<Tracked>();     // 1 аллокация, исключения-safe
std::unique_ptr<Tracked> q(new Tracked);  // то же, но виден «сырой» new
```

`make_unique`: нет строки с `new` → нет окна между выделением и взятием во владение;
аргументы передаются идеально (лекция 14); вложенные вызовы безопасны.
**В C++11 `make_unique` нет** (проверено) — `make_shared` появился в C++11, «младший брат» — только в C++14.

### 3б. Массивы

```cpp
auto arr = std::make_unique<int[]>(10);   // unique_ptr<int[]>: value-init (обнулён), удалится delete[]
arr[3] = 42;                              // operator[] есть
std::unique_ptr<int[]> pa(new int[5]);
```

`p->x` для `unique_ptr<T[]>` **не компилируется** (проверено) — массив нельзя
«разыменованием» превратить в элемент; для одиночного объекта обратно — нет `p[i]`.

### 3в. Custom deleter: `FILE*`, ручки SDK, возврат в пул

```cpp
void cfree(FILE* f) { if (f) std::fclose(f); }
std::unique_ptr<FILE, void(*)(FILE*)> f(std::fopen("x", "w"), &cfree);   // fclose вместо delete

struct Stateless { void operator()(Tracked* p) const { delete p; } };    // deleter без состояния
std::unique_ptr<Tracked, Stateless> p(new Tracked);
```

> **Какого размера `unique_ptr` с делетером?** Зависит от делетера (проверено, gcc 11):
> `default_delete<T>`/`struct` без полей — **8** (EBO), указатель на функцию — **16**,
> `std::function` — **40**. Делетер — **часть типа**: `unique_ptr<T, A>` и `unique_ptr<T, B>`
> — разные типы (как «разные аллокаторы = разные типы контейнера», лекция 12).

### 3г. `reset` и `release`

```cpp
p.reset();            // освободить то, чем владею (вызвать deleter), указатель обнулить
p.reset(newobj);      // старое освободить, принять новое
Tracked* raw = p.release();   // ОСВОБОДИТЬ ВЛАДЕНИЕ: вернул сырой указатель, сам обнулился
delete raw;                   // теперь это MY проблема
```

> **`release` = «забираешь сырой указатель на себя».** После него **никто** не освобождает:
> ни объект, ни сборщик (его нет). `reset` — наоборот, «я отдаю, освободи».

### 3д. Фабрика: конструктор закрыт, наружу `unique_ptr`

Прямая отсылка к лекции 11 («конструктор держать закрытым», многошаговую инициализацию —
в фабричную функцию):

```cpp
class Connection {
    int fd_;
    explicit Connection(int fd) : fd_(fd) {}       // приватный: создают только изнутри
public:
    static std::unique_ptr<Connection> create(int port) {
        if (port <= 0) throw std::invalid_argument("port");
        return std::unique_ptr<Connection>(new Connection(port));
    }
    ~Connection() { /* disconnect */ }
};
auto c = Connection::create(8080);      // бросок — утечки нет: ресурс сразу в unique_ptr
```

> `make_shared`/`make_unique` через приватный конструктор **не проходят** — смысл фабрики:
> «кривой» объект не создать. Возврат `unique_ptr` из функции — единственный способ вернуть
> некопируемый объект (NRVO/move — на плановой **лекции 14**).

### 3е. `unique_ptr` в сигнатурах

| Сигнатура | Что означает |
|---|---|
| `void take(std::unique_ptr<C> p)` | **«я ЗАБИРАЮ владение»**, вызывающий обязан `std::move` |
| `void use(C& c)` / `void use(C* c)` | «одалживаю», не владею |
| `std::unique_ptr<C> make()` | «владение уезжает наружу» (move/NRVO) |
| `v.push_back(std::move(p))` | контейнер — владелец; `push_back(p)` — **не скомпилируется** |

Полиморфное хранение (связка с лекцией 10, таблица CRTP → «Хранение в контейнере»):

```cpp
std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>());
shapes.push_back(std::make_unique<Square>());   // ~Circle, ~Square отработают (проверено)
```

> `vector<Shape>` с полиморфными объектами невозможен (срезка), CRTP-объекты разного типа
> в один контейнер не кладутся → нужен `vector<unique_ptr<Base>>` (лекция 10).

### 3ж. «Почему frame — `shared_ptr`, а кэш — `unique_ptr`?»

Вопрос ревьюера из `vacancy_tasks/README.md`. Ответ — через **число владельцев**:

```cpp
struct Trajectory {
    std::shared_ptr<const Frame> frame;                 // общих read-only координат — МНОГО владельцев
    std::unique_ptr<std::vector<double>> cached_len;    // кэш: владелец РОВНО ОДИН
    const std::vector<double>& lengths() {
        if (!cached_len) cached_len = std::make_unique<std::vector<double>>();   // ленивая инициализация
        return *cached_len;
    }
};
```

- **frame** — общие неизменяемые данные, на них смотрят все траектории партии →
  **разделённое владение** → `shared_ptr<const Frame>` (проверено: `use_count == 3`);
- **кэш** — часть состояния одной траектории: второго хозяина не бывает, копия кэша
  бессмысленна, а `shared_ptr` заплатил бы контрольным блоком и атомарными инкрементами
  на каждое перемещение → **`unique_ptr`**. При `std::move` траектории кэш **переезжает**
  (проверено: `t1.cache=0`, `t3.cache=1`) — гонок нет.

> Пункт ревьюера «потокобезопасность ленивого кэша»: `if (!cached) cached = …` в двух
> потоках — гонка. Лечится мьютексом (RAII-`scoped_lock`, **лекция 21**), а не типом указателя.

---

## 4. `shared_ptr` — разделённое владение

### 4а. Контрольный блок

> **Прямой ответ на 13-ю лекцию** («нужно хранить указатель + счетчик»): да, но счётчик
> живёт **не в объекте**, а в отдельном **контрольном блоке**:

```
shared_ptr (16 байт): [ T* p_ | CtlBlock* ]
CtlBlock: { strong_count, weak_count, deleter, allocator }
```

- два указателя → `sizeof(shared_ptr) == 16`, `sizeof(weak_ptr) == 16`, `sizeof(unique_ptr) == 8` (проверено);
- счётчик **не в объекте** (это **не** intrusive-подход): работает с чужими классами и массивами;
- операции со счётчиком **атомарные** (требование стандарта) — инкремент/декремент из
  нескольких потоков безопасны;
- **deleter и allocator запоминаются в момент взятия владения** (запомнится статический тип!).

### 4б. Конструктор копирования = инкремент счётчика

Ответ на «как должен выглядеть конструктор копирования?» из лекции 13 (там же «через
`deepcopy()`»):

```cpp
shared_ptr(const shared_ptr& o) noexcept : p_(o.p_), ctl_(o.ctl_) { ++ctl_->strong; }
shared_ptr& operator=(const shared_ptr& o) noexcept { /* инкремент нового + декремент старого */ }
```

> **`deepcopy()` не нужен.** Копия `shared_ptr` — **копия указателя + инкремент счётчика**:
> O(1), объект не копируется вообще. Глубокая копия в 13-й лекции была нужна потому, что
> указатель владел памятью эксклюзивно (свой `StackAllocator`); с появлением
> **разделённого владения** deepcopy избыточен.

```cpp
auto a = std::make_shared<T>("a");
{ auto b = a; auto c = b; auto e = std::move(c); }
// a=3, b=3, c=0 (перемещён), e=3; после блока a=1            (проверено)
```

`use_count()` — число **сильных** владельцев; `unique()` == `use_count() == 1` (в C++20
устарело: «был один» нельзя проверить атомарно).

### 4в. Порядок уничтожения

Последний владеющий вызывает **deleter** (он помнит статический тип на момент
конструирования блока), затем по цепочке уничтожаются поля/базы. Локальные объекты и
элементы контейнера уничтожаются **в обратном порядке** конструирования (проверено: два
`shared_ptr` в массиве → `~y ~x`).

### 4г. `shared_ptr<T>(new T)` = 2 аллокации, `make_shared` = 1

Посчитано счётчиком аллокаций через глобальный `operator new` (проверено):

| Выражение | Аллокаций |
|---|---|
| `std::make_unique<T>()` | **1** |
| `std::shared_ptr<T>(new T)` | **2** (объект + блок) |
| `std::make_shared<T>()` | **1** (объект и блок одним куском) |
| `std::shared_ptr<T>(new T, лямбда)` | **2** |

Плюсы `make_shared`: один кусок (локальность, меньше фрагментации), нет «окна» между
`new` и взятием владения, исключение безопасно.

> **Ловушка `make_shared`:** объект и блок лежат **одним куском**, поэтому **память
> объекта не освободится, пока жив хоть один `weak_ptr`** — даже когда `use_count() == 0`
> и деструктор уже отработал. Проверено: после `p.reset()` → `expired=1`, `use_count=0`,
> `lock() → nullptr`, а кусок (в примере 1 КБ) ещё занят.
> Лечится: `shared_ptr<T>(new T)` (объект освобождается сразу) или не держать `weak_ptr`.

### 4д. `allocate_shared` — связка с аллокаторами (лекция 12)

```cpp
template<class T> struct MyAlloc {
    using value_type = T;
    T* allocate(std::size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    void deallocate(T* p, std::size_t n) noexcept { (void)n; ::operator delete(p); }
    template<class U> MyAlloc(const MyAlloc<U>&) noexcept {}
};
auto p = std::allocate_shared<S>(MyAlloc<S>{}, 42);   // блок+объект — через МОЙ аллокатор (проверено: 1 allocate)
```

> Ровно «та же тема выделения памяти «под ключ»» из 12-й лекции: `allocate_shared`
> принимает аллокатор и строит им **объект и контрольный блок**.
> `make_shared` = `allocate_shared(std::allocator<T>, ...)`.

### 4е. `enable_shared_from_this` и `weak_from_this`

Готовый пример **CRTP из лекции 10**; внутри — **слабый** указатель на себя:

```cpp
struct Widget : std::enable_shared_from_this<Widget> {
    std::shared_ptr<Widget> self() { return shared_from_this(); }
};
auto w = std::make_shared<Widget>();
auto s = w->self();       // use_count = 2 — ТОТ ЖЕ контрольный блок, а не новый
```

- ctor `enable_shared_from_this` **не знает** адрес объекта; `shared_ptr` **подписывает**
  weak-указатель на себя в момент взятия владения;
- `shared_from_this()` у объекта, **которым ещё не владеет** `shared_ptr`, — **UB**
  (gcc 11 бросает `std::bad_weak_ptr`, вне `try` → `std::terminate`; проверено);
- `weak_from_this()` (**C++17**) — безопасная версия: вернёт пустой `weak_ptr`
  (проверено: `use_count == 0`), исключений нет.

> Никогда не делай `shared_ptr<T>(this)` внутри метода → **второй контрольный блок** →
> два счётчика → **double free** (проверено: процесс падает на `double free detected`).

### 4ж. Конверсии, полиморфный `delete`, виртуальный деструктор

```cpp
std::shared_ptr<Derived> d = std::make_shared<Derived>();
std::shared_ptr<Base> b = d;              // ЕСТЬ: конверсия «вверх» = полиморфизм
std::shared_ptr<Derived> d2(b);           // НЕ компилируется
auto d3 = std::static_pointer_cast<Derived>(b);   // C++17; есть и dynamic_pointer_cast
```

Полиморфное удаление (прямая связка с **лекцией 8 — виртуальный деструктор**):

```cpp
struct Base    { ~Base() {} };                              // НЕ virtual!
struct Derived : Base { int* y = new int[4]; ~Derived() { delete[] y; } };

std::shared_ptr<Base> p(new Derived);   // ~Derived ВЫЗОВЕТСЯ, утечки нет (проверено + ASan чист)
std::unique_ptr<Base> u(new Derived);   // UB: ASan ловит new-delete-type-mismatch
```

> **Почему так:** `shared_ptr` пишет в контрольный блок **делетер от `Derived`** (тип,
> с которым забрали владение), и тот знает полный размер и полный деструктор —
> **делетер «съедает» необходимость виртуального деструктора**.
> У `unique_ptr` делетер — `default_delete<Base>`, он помнит только `Base` → `delete` по
> базе без `virtual ~Base()` = UB + утечка полей потомка (правило 8-й лекции в силе).

Мелочи, которые спрашивают: `std::get_deleter<D>(p)` — достать свой делетер (для лямбды
не сработает: тип делетера — замыкание, проверено); aliasing-конструктор
`std::shared_ptr<T> alias(owner, &owner->payload)` — владеем `owner`, указываем на часть
(proверено: `use_count` владельца растёт); `shared_ptr<void>` — стёртый тип (делетер из
блока всё равно правильный).

---

## 5. Циклические ссылки и `weak_ptr`

### 5а. Утечка по схеме счётчиков

```cpp
struct Node { std::shared_ptr<Node> next; ~Node() { std::puts("~N"); } };
auto a = std::make_shared<Node>();
auto b = std::make_shared<Node>();
a->next = b;                 // b.use_count = 2
b->next = a;                 // a.use_count = 2   → печатает «2 2» (проверено)
```

```
a ──strong──► [block a, strong=2] ◄──next── [Node b]
b ──strong──► [block b, strong=2] ◄──next── [Node a]
```

Конец области: каждый отпустил по одной ссылке → **strong = 1 у обоих** → ноль **не
достигнут** → контрольные блоки не сработали → **деструкторы не вызваны, память утекла**
(проверено: `~N` не печатается ни разу).

### 5б. `weak_ptr`

`weak_ptr` — **владелец нулевой силы**: держит **только контрольный блок**, strong-счётчик
не двигает.

```cpp
std::weak_ptr<Node> w = a;
if (auto sp = w.lock()) { /* use_count стал 2, объект точно жив */ }
if (w.expired()) { /* объект умер */ }
std::printf("strong=%ld weak=%ld\n", a.use_count(), w.use_count());
```

- **`lock()`** — атомарно «поднимаю» до `shared_ptr`, если объект жив, иначе `nullptr` (проверено);
- **`expired()`** — `true` надёжно; `false` **не гарантирует** жизнь (между проверкой и
  `lock()` объект могут отпустить) → **всегда через `lock()`**;
- `use_count()`/`weak_count()` у `weak_ptr` — есть; **у `shared_ptr` метода `weak_count()`
  нет** (проверено: нет такого члена);
- `operator*`/`operator->` у `weak_ptr` **нет** (проверено: не компилируется).

Цикл чинится одной строкой (проверено: оба `~Fixed` отработали):

```cpp
struct Fixed { std::shared_ptr<Fixed> next; std::weak_ptr<Fixed> obs; };
a->next = b;
b->obs  = a;      // обратная ссылка НЕ владеет → цикл разомкнут
```

### 5в. Где `weak_ptr` — нативный житель

- **observer / подписчик**: слушатель не должен продлевать жизнь издателю;
- **кэш** `std::map<Id, std::weak_ptr<Object>>`: кэш **не владеет**, `lock()` = попадание,
  `nullptr` = выкинуть запись (проверено: после смерти владельца `expired=1`, `lock() → MISS`);
- родитель ↔ ребёнок: в одну сторону `shared_ptr`, в другую — `weak_ptr`.

> Снова вопрос ревьюера: **frame — `shared_ptr`** (реально много владельцев),
> **кэш — `unique_ptr`** (один владелец), а если кэш смотрит на чужие объекты —
> **`weak_ptr`** (не продлевать жизнь). **Тип указателя = заявление о владении.**

---

## 6. Правила владения (формулировки для собеса)

### 6а. Сырой указатель/ссылка = наблюдатель

```cpp
void use(Shape* s);        // одалживаю; владелец — вызывающий
void use(Shape& s);        // то же, когда null не бывает
```

Ссылка `T&` владеть не может физически: она не перемещается и не обнуляется.

### 6б. Никогда не `shared_ptr<T>(new T)` в аргументах функции

```cpp
f(std::shared_ptr<A>(new A), g());   // порядок вычисления аргументов НЕ определён
f(std::make_shared<A>(), g());       // правильно: ресурс сразу в объекте, бросок безопасен
```

Возможная интерпретация: `new A` выполнен → `g()` бросает → **`shared_ptr` ещё не создан,
его деструктор нечего вызывать → утечка**. Детерминированный вариант (сырой указатель
живёт до `shared_ptr`) ASan ловит: `SUMMARY: AddressSanitizer: 1 byte(s) leaked`.

### 6в. `const shared_ptr<T>` против `shared_ptr<const T>`

| Объявление | Константен | `p->x = 1;` | `p.reset();` |
|---|---|---|---|
| `const std::shared_ptr<T> p` | **указатель** | ✅ компилируется | ❌ не компилируется |
| `std::shared_ptr<const T> p` | **объект** | ❌ не компилируется | ✅ компилируется |

(обе «строки-ошибки» проверены). `shared_ptr<const T>` — «разделяемое read-only», идеален
для того самого `frame`.

### 6г. Потокобезопасность

- **счётчик** контрольного блока — **атомарный**: два потока могут делать копии/`=`/`reset`
  **своих** экземпляров указателя;
- **сам объект** — **нет**: два потока в `p->field` = data race, нужен мьютекс (**лекция 21**);
- менять **один и тот же** объект-указатель из двух потоков — гонка:
  `std::atomic<std::shared_ptr<T>>` — **C++20** (в gcc 11 отсутствует:
  `static assertion failed: std::atomic requires a trivially copyable type`);
  `std::atomic_load/atomic_store(&p)` — **C++11**, работает и сегодня, **устарел в C++20**,
  **удалён в C++26**;
- `use_count()` в многопоточной программе — отладочная информация, не условие логики.

---

## 7. Реализация руками (обязательный домашний код)

### 7а. `IntrusivePtr` — счётчик ВНУТРИ объекта

Ровно то, что сформулировано в 13-й лекции: «хранить указатель + счётчик». Здесь счётчик
лежит в объекте (intrusive-подход):

```cpp
struct RefCounted { mutable size_t refs = 0; virtual ~RefCounted() = default; };

template<class T>
class IntrusivePtr {
    T* p_ = nullptr;
    void destroy() noexcept { if (p_ && --p_->refs == 0) delete p_; }
public:
    IntrusivePtr() = default;
    explicit IntrusivePtr(T* p) : p_(p) { if (p_) ++p_->refs; }
    IntrusivePtr(const IntrusivePtr& o) : p_(o.p_) { if (p_) ++p_->refs; }   // копия = инкремент
    IntrusivePtr& operator=(const IntrusivePtr& o) { IntrusivePtr(o).swap(*this); return *this; }
    IntrusivePtr(IntrusivePtr&& o) noexcept : p_(o.p_) { o.p_ = nullptr; }
    IntrusivePtr& operator=(IntrusivePtr&& o) noexcept {
        if (this != &o) { destroy(); p_ = o.p_; o.p_ = nullptr; }
        return *this;
    }
    ~IntrusivePtr() { destroy(); }
    T& operator*()  const noexcept { return *p_; }         // унарные * и -> — лекция 6
    T* operator->() const noexcept { return p_; }
    T* get()        const noexcept { return p_; }
    explicit operator bool() const noexcept { return p_ != nullptr; }
    size_t use_count() const noexcept { return p_ ? p_->refs : 0; }
    void swap(IntrusivePtr& o) noexcept { std::swap(p_, o.p_); }
    void reset() noexcept { T* p = p_; p_ = nullptr; if (p && --p->refs == 0) delete p; }
};
```

Цена: `sizeof == 8`, **ноль** дополнительных аллокаций, копия — один инкремент.
Расплата: класс **обязан** наследовать `RefCounted` (чужой класс не обернём), счётчик виден наружу.

### 7б. `RefCntPtr` — счётчик в отдельном блоке (как настоящий `shared_ptr`)

```cpp
struct CtlBlock { size_t strong = 0; size_t weak = 0; };

template<class T>
class RefCntPtr {
    T* p_ = nullptr;
    CtlBlock* ctl_ = nullptr;
    void acquire() noexcept { if (ctl_) ++ctl_->strong; }
    void release() noexcept {
        if (!ctl_) return;
        if (--ctl_->strong == 0) { delete p_; if (ctl_->weak == 0) delete ctl_; }
        p_ = nullptr; ctl_ = nullptr;
    }
public:
    RefCntPtr() = default;
    explicit RefCntPtr(T* p) : p_(p), ctl_(new CtlBlock{1, 0}) {}
    RefCntPtr(const RefCntPtr& o) noexcept : p_(o.p_), ctl_(o.ctl_) { acquire(); }
    RefCntPtr& operator=(const RefCntPtr& o) noexcept { RefCntPtr(o).swap(*this); return *this; }
    RefCntPtr(RefCntPtr&& o) noexcept : p_(o.p_), ctl_(o.ctl_) { o.p_ = nullptr; o.ctl_ = nullptr; }
    RefCntPtr& operator=(RefCntPtr&& o) noexcept { RefCntPtr(std::move(o)).swap(*this); return *this; }
    ~RefCntPtr() { release(); }
    T& operator*()  const noexcept { return *p_; }
    T* operator->() const noexcept { return p_; }
    size_t use_count() const noexcept { return ctl_ ? ctl_->strong : 0; }
    bool unique() const noexcept { return use_count() == 1; }
    void swap(RefCntPtr& o) noexcept { std::swap(p_, o.p_); std::swap(ctl_, o.ctl_); }
    void reset() noexcept { release(); }
};
```

Проверено: `ctor == dtor`, ASan чист, `sizeof(RefCntPtr) == 16` (как у `shared_ptr`),
`sizeof(IntrusivePtr) == 8`.

Что важно увидеть глазами:
- **`= ` через «копия + swap»** — безопасно при self-assignment и даёт **strong guarantee**:
  старое освобождается в деструкторе временного объекта (copy-and-swap из лекции 11);
- **move не трогает счётчик** — только обнуляет источник (два указателя = 16 байт, swap за O(1));
- **`swap` обязателен**: он `noexcept`, им сделаны и `= `, и сильная гарантия.

### 7в. Что в этом решении ломается

| Проблема | Почему | Куда идти |
|---|---|---|
| Потоки | `++ctl_->strong` — обычный `size_t`, гонка | `std::atomic<size_t>` (лекция 21), `std::atomic<std::shared_ptr>` (C++20) |
| Циклы | `a->next=b; b->next=a` → счётчик не обнулится | `weak_ptr` + `lock()` (п. 5) |
| Исключение в ctor | блок создан, объект нет → блок утекает | `make_shared`/`allocate_shared` (блок+объект одной аллокацией) |
| Полиморфный `delete` | `delete p_` по статическому типу | делетер в блоке (п. 4ж) + виртуальный деструктор (лекция 8) |
| Массивы | нужен `delete[]` | `default_delete<T[]>` / свой делетер |

---

## 8. Сводная таблица

| Тема | `T*` (сырой) | `unique_ptr` | `shared_ptr` | `weak_ptr` | `T&` | `string_view` |
|---|---|---|---|---|---|---|
| Аллокаций на создание | 0 (свой `new` = 1) | 1 (`make_unique`) | 1 (`make_shared`) / 2 (`(new T)`) | 0 | 0 | 0 |
| `sizeof` | 8 | 8 (stateless deleter) / 16 / 40 | 16 | 16 | 8 | 16 |
| Цена копии | 8 байт, ничего не освобождает | **копии нет** (move) | инкремент счётчика, O(1) | инкремент weak-счётчика | нет копии (нет перепривязки) | 16 байт |
| Владение | **никакое** | **эксклюзивное** | **разделённое** | **наблюдение** | наблюдение | наблюдение |
| Поток: счётчик | — | — | **атомарен** | атомарен | — | — |
| Поток: сам объект | — | — | **нет**, нужен мьютекс | нет | нет | нет (const) |
| Плата за «умность» | — | тип делетера | контрольный блок + 8 байт | контрольный блок | — | — |
| Главный риск | утечка / double free | забыть `std::move` | **цикл ссылок** | `expired`, гонка с `lock()` | висячая ссылка | висячие данные |

Правило выбора в одну строку: **один владелец → `unique_ptr`; владельцев несколько →
`shared_ptr`; не владелец → сырой указатель/ссылка/`weak_ptr`; владельца нет вообще →
`optional`/значение.**

---

## 9. `std::optional` / `std::variant` (C++17): владение без `new`

Умный указатель отвечает на «кто освобождает память». Часто память под `new` не нужна
вовсе — нужен «значение или ничего».

```cpp
std::optional<S> o;                     // объект ВНУТРИ optional, на стеке: аллокаций 0
o.has_value();                          // false
o.emplace(3);                           // конструктор «на месте»
o->v;                                   // 3
try { std::optional<S> e; (void)e.value(); }
catch (const std::bad_optional_access& e) {}    // value() на пустом бросает
std::variant<S, int> var = 7;           // «одно из»: index(), std::get, bad_variant_access
```

- **`has_value()`/`operator bool`** — проверка; **`operator*`/`operator->`** — быстрый путь
  (UB при пустом); **`value()`** — проверка и бросок **`std::bad_optional_access`**,
  производного от `std::exception` (иерархия исключений из лекции 12);
- `sizeof(optional<S>) == sizeof(S) + флаг` (проверено: для `S{int}` — 8 байт, padding съел);
- «**опциональность без указателя**»: `shared_ptr<T>` как «может не быть» — это контрольный
  блок, `use_count` и соблазн передать владение дальше; `optional<T>` говорит «может не быть
  значения» и **никого не делает владельцем**. Связка с лекцией 11: ожидаемые ситуации
  (не нашёл) — `optional`/код ошибки, редкие аварии — исключения.

---

## 10. Экзаменационные «что будет» (в стиле Мещерина)

### 10а. Посчитать `use_count`

```cpp
auto a = std::make_shared<T>("a");
{ auto b = a; auto c = b; auto e = std::move(c);
  std::printf("%ld %ld %ld %ld\n", a.use_count(), b.use_count(), c.use_count(), e.use_count()); }
std::printf("%ld\n", a.use_count());
```

**Ответ: `3 3 0 3`, потом `1`.** `a`/`b`/`e` — три владельца; `c` **перемещён** в `e`,
у move-операций инкремента нет. Проверено.

### 10б. Цикл ссылок: сколько деструкторов?

```cpp
struct Node { std::shared_ptr<Node> next; ~Node() { std::puts("~N"); } };
{ auto a = std::make_shared<Node>(); auto b = std::make_shared<Node>();
  a->next = b; b->next = a; }
```

**Ответ: 0 (ноль) `~N`, утечка.** В конце области счётчики 1 и 1 (внутри печатается `2 2`,
проверено). Починка: `std::weak_ptr<Node> next;` (или `weak_ptr` на обратную ссылку) →
оба деструктора отработают.

### 10в. `make_shared` + долгий `weak_ptr`

```cpp
struct Big { char buf[1024]; ~Big() { std::puts("~Big"); } };
std::weak_ptr<Big> w;
{ auto p = std::make_shared<Big>(); w = p; }
std::printf("%d %ld\n", (int)w.expired(), w.use_count());
auto l = w.lock();
```

**Ответ: `1 0`; `lock()` → `nullptr`; деструктор отработал, а 1024 байта ЗАНЯТЫ** —
`make_shared` выделил объект и блок одним куском, кусок жив, пока жив последний
наблюдатель (проверено). «Починить»: `std::shared_ptr<Big>(new Big)` (объект освобождается
сразу, блок отдельно) или не держать `weak_ptr` дольше объекта.

### 10г. Два `shared_ptr` с одного «сырого» указателя

```cpp
Res* raw = new Res{1};
std::shared_ptr<Res> a(raw);
std::shared_ptr<Res> b(raw);
```

**Ответ: два независимых контрольных блока → `use_count` у обоих `1`, потом двойное
удаление** (проверено: процесс падает на `double free detected`). Правильно: один
`shared_ptr` + его копия, либо `enable_shared_from_this`.

---

## За рамками лекции (что доучить)

- **`std::atomic<std::shared_ptr<T>>` / `std::atomic<std::weak_ptr<T>>` (C++20)** — законный
  способ менять сам указатель из нескольких потоков. **В gcc 11 не реализовано**:
  `static assertion failed: std::atomic requires a trivially copyable type`.
- **`std::atomic_load/store/exchange/compare_exchange` для `shared_ptr` (C++11)** — рабочий
  способ сегодня (проверено), **устарели в C++20**, **удалены в C++26**; реализованы обычно
  глобальной таблицей мьютексов по адресу блока.
- **`weak_from_this()` (C++17)** — безопасная замена `shared_from_this()`; `shared_from_this()`
  у объекта вне владения — UB (gcc бросает `std::bad_weak_ptr`).
- **`std::enable_shared_from_this<T>` изнутри**: там лежит **weak**-счётчик, который
  `shared_ptr` «подписывает» при взятии владения; два `shared_ptr` с одного адреса —
  double free (мост к плановой лекции 21: «`atomic<shared_ptr>` — плохая идея»).
- **`std::atomic`/`std::scoped_lock` как «RAII для мьютекса»** — тот же приём: acquire в ctor,
  release в dtor; `scoped_lock` (C++17), `std::lock` от дедлока, `unique_lock` — мост к
  **лекции 21** (там же `multithreaded_programming/test.cpp`, который держит
  `shared_ptr`/`promise`/`atomic<bool>`).
- **`std::allocate_shared` со своим аллокатором** (в т. ч. `pmr`): один кусок под
  объект+блок в нужной памяти; `make_shared` = `allocate_shared(std::allocator<T>, …)`
  (связка с лекцией 12, «выделение памяти под ключ»).
- **Массивы в `shared_ptr`**: `shared_ptr<T[]>` (C++17) с `operator[]` есть, а
  **`make_shared<T[]>` — нет** (`static_assert: make_shared<T[]> not supported`);
  `shared_ptr<T>(new T[10])` компилируется и является UB (делетер не тот).
- **Книги** (`literature/`): *Effective C++* — «не отдавать `.get()` наружу», виртуальный
  деструктор; *Эффективный и современный C++* (Meyers) — **item 18–22**: `make_unique`,
  «не передавай и не возвращай `unique_ptr` зря», «`shared_ptr` по значению»,
  `atomic_*`-функции; *C++17 STL* — `optional`/`variant`/`string_view`; *N4860* —
  `[util.smartptr]`; *Параллельное программирование на C++* (Williams) — что именно
  атомарно в контрольном блоке.

> **Задача после лекции:** написать свои `shared_ptr`/`unique_ptr` (ctor, копия, move,
> dtor, `operator*`/`operator->`, `swap`, `reset`, custom deleter, массивы), покрыть
> тестами (self-assignment, цикл ссылок, два блока с одного адреса,
> `enable_shared_from_this`) и проверить **`-fsanitize=address`** — счётчики
> `ctor == dtor` и `alloc == free` обязаны сходиться.

---

## Короткая шпаргалка

| Тема | Главное |
|---|---|
| **RAII** | ресурс acquired в ctor, **освобождается в dtor**; время жизни ресурса = время жизни объекта |
| **Бросок из ctor** | объект не достроен → **его деструктор не вызывается** → всё «ручное» утекло (лекция 11) |
| **Почему RAII спасает** | деструкторы локальных объектов вызываются при **любом** выходе из области (unwinding) |
| **GC** | в C++ **нет**; отвечают деструкторы |
| **Деструктор** | обязан быть **noexcept**: `try { … } catch (...) {}` |
| **Умный указатель (лекция 6)** | объект, хранящий указатель и управляющий жизненным циклом; держит `*` и `->` |
| **`auto_ptr`** | «копия» обнуляет источник; `const auto_ptr&` не компилируется; ломает `vector`; **удалён в C++17** |
| **`unique_ptr<T, D>`** | move-only, копирования нет; `D` — **часть типа**; размер 8 / 16 / 40 |
| **`make_unique`** | **C++14** (в C++11 нет); 1 аллокация; безопасно при исключениях |
| **`unique_ptr<T[]>`** | `delete[]`, есть `operator[]`, **`p->x` не компилируется** |
| **`reset` / `release`** | `reset` — «освободи»; `release` — **отдать владение**, вернуть сырой указатель → `delete` на мне |
| **Фабрика** | `static std::unique_ptr<C> create()` + приватный ctor (лекция 11); `make_shared` не пройдёт |
| **`unique_ptr` в параметре** | «я забираю владение», вызывающий пишет `std::move` |
| **`vector<unique_ptr<Base>>`** | полиморфные объекты в контейнере (лекция 10, таблица CRTP) |
| **frame / кэш** | frame — `shared_ptr` (много владельцев), кэш — `unique_ptr` (один), кэш на чужое — `weak_ptr` |
| **Контрольный блок** | `strong`, `weak`, deleter, allocator; счётчик **не в объекте**; **атомарный** |
| **`sizeof`** | `unique_ptr` 8, `shared_ptr` **16**, `weak_ptr` **16** |
| **Копия `shared_ptr`** | **копия указателя + инкремент счётчика**, O(1); **`deepcopy()` не нужен** (ответ 13-й лекции) |
| **`use_count` / `unique`** | число сильных владельцев / `==1` (`unique` устарело в C++20) |
| **Порядок уничтожения** | последний владеющий → deleter; локальные объекты — **обратно** порядку создания |
| **Аллокации** | `shared_ptr(new T)` — **2**; `make_shared` — **1** |
| **Ловушка `make_shared`** | память объекта занята, **пока жив хоть один `weak_ptr`** |
| **`allocate_shared`** | строит объект+блок **своим аллокатором** (лекция 12) |
| **`enable_shared_from_this`** | CRTP (лекция 10), внутри **weak**; `shared_from_this()` вне владения = **UB** |
| **`weak_from_this()`** | **C++17**, безопасная версия (пустой `weak_ptr`, без исключения) |
| **Два `shared_ptr(raw)`** | два контрольных блока → **double free** |
| **Конверсии** | `shared_ptr<Derived>`→`shared_ptr<Base>` есть; обратно — `static_pointer_cast` (C++17) |
| **Полиморфный delete** | `shared_ptr<Base>(new Derived)` без `virtual ~Base` — **~Derived вызовется** (делетер помнит `Derived`) |
| **`unique_ptr<Base>` без `virtual ~Base`** | **UB** (ASan: new-delete-type-mismatch) — виртуальный деструктор обязателен (лекция 8) |
| **Цикл ссылок** | `a→b`, `b→a`: strong = 1/1, **ноль деструкторов**, утечка |
| **`weak_ptr`** | не владеет; `lock()` / `expired()` / `use_count()` / `weak_count()`; **нет `*` и `->`** |
| **`shared_ptr` не имеет** | метода `weak_count()` |
| **Сырой `T*` / `T&`** | **наблюдатель**, владения ноль |
| **`shared_ptr<T>(new T)` в аргументах** | **запрещено**: порядок вычисления аргументов не определён + бросок = утечка; пиши `make_shared` |
| **Передача** | `const shared_ptr<T>&` (копия = инкремент); по значению — только если забираешь владение |
| **`const shared_ptr<T>` / `shared_ptr<const T>`** | константен **указатель** (объект менять можно) / константен **объект** (писать нельзя) |
| **Поток** | счётчик — **да**; **объект — нет**; один указатель из двух потоков — `atomic<shared_ptr>` (C++20) / `atomic_load/store` (C++11, устарели) |
| **IntrusivePtr** | счётчик в объекте: `sizeof 8`, 0 аллокаций, нужен базовый `RefCounted` |
| **RefCntPtr** | счётчик в блоке: `sizeof 16`; `=` через «копия+swap», move не трогает счётчик |
| **`optional`/`variant`** | владение **без `new`**: `has_value()`, `value()` → `bad_optional_access` (лекция 12) |
