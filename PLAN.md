# План курса: разбор лекций 1–13 и что дальше

> Задача: курс обрывается на середине 13-й лекции (контейнеры), и дальше непонятно, что будет.
> Ниже — (1) что реально пройдено, (2) какие нитки оборваны и ведут в будущее, (3) план лекций 14+.

---

## 1. Карта пройденного (1–13)

| # | Тема | Ключевое | Статус |
|---|---|---|---|
| 1 | Введение в язык | `test.cpp` — только комментарий | пусто (только код-заготовка) |
| 2 | Модификаторы типов, указатели | `*`/`&`, `int**`, размер указателя, карта памяти | пусто (readme 0 байт) |
| 3 | Модификаторы типов | `const`, ссылки, `typedef` | пусто |
| 4 | Модификаторы типов, введение в ООП | класс, конструктор, деструктор, конструктор копирования (об этом говорит начало 5-й: «на прошлой лекции 2а/2б/2в») | пусто |
| 5 | Классы, перегрузка операторов (начало) | `this`, `.`/`->`, `operator=`, Rule of 3, списки инициализации, `explicit`, `const`-методы, `mutable`, `friend`, делегирующие ctor, `static`, указатели на члены, `+`/`+=` | ✅ final_readme + questions |
| 6 | Перегрузка операторов (продолжение) + наследование (начало) | операторы вне класса, `<<`/`>>`, сравнения, `++i`/`i++`, `[]`, `operator()` (функтор), унарные `* & ->`, «что нельзя перегрузить», операторы преобразования; 3 вида наследования | ✅ |
| 7 | Наследование | hiding, `Base::f()`, порядок ctor/dtor, множественное и виртуальное наследование, размер объекта, slicing, касты вверх/вниз | ✅ |
| 8 | Наследование + полиморфизм | `static_cast`/`reinterpret_cast`/`dynamic_cast`, `virtual`, виртуальный деструктор, vtable, абстрактные классы, `override`/`final`, `typeid` | ✅ |
| 9 | Шаблоны (начало) | `template<class T>`, специализация, `typedef`/`using`, `typename`, type traits (`remove_const`/`remove_reference`), вывод аргументов `T&`/`T`, non-type template params, шаблоны-шаблоны | ✅ |
| 10 | Шаблоны (прод.) + исключения (гл. 7, начало) | variadic templates, `sizeof...`, пакет в типах, функторы/компараторы, **CRTP**, `new`/`bad_alloc`, `throw/try/catch`, правило порядка `catch` | ✅ |
| 11 | Исключения (гл. 7, продолжение) | исключение = копия (счёт конструкторов), `throw;` vs `throw f;` (slicing), старые `throw(...)`, `noexcept` + оператор `noexcept`, исключения в ctor (утечка) и dtor (`terminate`), гарантии безопасности, **первое появление RAII и умных указателей** | ✅ |
| 12 | `operator new`/`delete`, аллокаторы (гл. 8) | глобальная/членская перегрузка, placement new, явный `p->~T()`, `nothrow`, `new_handler`, полиморфный `delete`, свой аллокатор, `allocator_traits`, `rebind`, `std::pmr` | ✅ |
| 13 | Контейнеры (гл. 9) | `vector` руками: `allocator_traits::allocate/construct/destroy`, реаллокация ×2, `push_back`, `pop_back`, `emplace_back`, `[]`/`at`, ctor/dtor/`=`, `swap`, `shrink_to_fit`, 2 экзаменационных вопроса | ⚠️ **обрывается**: нет `final_readme.md`, нет `questions.md` |

Логика курса читается однозначно: **Мещерин идёт по главам учебника + Effective C++ Мейерса**
(в конспектах прямо цитируются главы 7 — исключения, 8 — память/аллокаторы, 9 — контейнеры, 10 — наследование,
и правила Мейерса «правило 10», «не бросать в деструкторе», «виртуальный деструктор»).
Значит формат будущих лекций тот же: **низкоуровневый механизм → как это писать руками → вопрос на собес**.

---

## 2. Оборванные нитки: автор сам оставил ссылки на будущее

Это не догадки — прямые отсылки в конспектах:

| Где | Цитата | Что требует |
|---|---|---|
| лекция 13, `emplace_back` | «`template<class ... Args> void emplace_back(const Args& ... args);` **и это неправильно. суть в move семантике**» | move + perfect forwarding (14) |
| лекция 13, реаллокация | `traits::construct(alloc, newarr + i, arr[i]);` — «это неправильно, но для 2007 года норм, **надо делать через move**» | move-конструктор + `noexcept` |
| лекция 13, проблема подсчёта ссылок | «Нужно хранить указатель + счетчик… (**умные указатели**)» | `shared_ptr` (15) |
| лекция 13, задание | «напишите `list` с нестандартным аллокатором», `StackAllocator<int>`, «как должен выглядеть конструктор копирования?», «через `deepcopy()`» | свой `list` + stateful allocator (16) |
| лекция 11, п. 6а | «класс, который в конструкторе принимает указатель, а в деструкторе делает `delete` — **это умные указатели!!!**» | `unique_ptr` (15) |
| лекция 6, п. 7 | унарные `*`, `&`, `->` — «**Что такое умный указатель?**» (вопрос повис с 6-й лекции) | `operator*`/`operator->` (15) |
| лекция 5, Rule of 3 | «В C++11 вместо правила 3 есть **правило 5**» — и дальше по курсу оно ни разу не раскрыто | move-конструктор, move-`=`, `= default`/`= delete` (14) |
| лекция 12, п. 8а | «`std::allocate_shared`, `std::make_unique` — та же тема выделения памяти „под ключ“» | `allocate_shared` + deleter (15) |
| лекция 9, п. 6 | «Что такое итераторы?» — задан и не раскрыт; `C<T>::type` | итераторы (17) |
| лекция 8, п. 11.2 | «Грамотный способ: шаблоны» (вместо `typeid`) — не доведено | tag dispatch / `if constexpr` / SFINAE (20) |
| лекция 10, CRTP | «это бред, но надо разобраться», имитация виртуальных функций без `virtual` | CRTP + static polymorphism, интерфейсы (20) |
| лекция 12, п. 8 | «вектор должен всё делать через аллокатор» → свой контейнер не дописан | дописать `vector`/`list` целиком (16) |
| `vacancy_tasks/README.md` | ревьюер спрашивает: «Почему `noexcept` на move?», «Почему frame — `shared_ptr`, а кэш — `unique_ptr`?», «Что с moved-from объектом?», «Почему `Add(T)` принимает by value + move?» | это **дословно программа лекций 14–15** |
| `multithreaded_programming/test.cpp` | `std::thread`, `std::async`, `std::promise/future`, `std::atomic<bool>`, `thread_safe_stack`, `hardware_concurrency`, параллельная quicksort | модуль многопоточности (21–22), `literature/Параллельное программирование на C++.pdf` (Williams) |

**Вывод:** «дальше умные указатели и move семантика» — подтверждено текстом лекций.
Порядок должен быть **сначала move, потом умные указатели**: `shared_ptr` без move не объясним
(перемещение контрольного блока, `std::move` в `swap`, `make_shared`, rule of 5 в обёртке),
а `emplace_back` из 13-й лекции вообще невозможно «починить» без rvalue-ссылок.

---

## 3. План лекций 14+

### Блок A. Modern C++: значение и владение (главное, чего не хватает)

**Лекция 14. Value categories и move семантика**
- lvalue / xvalue / rvalue, glvalue/prvalue; правила «`&` на rvalue нельзя», почему `const&` продлевает жизнь
- rvalue-ссылка `T&&`, reference collapsing (`& &&`), почему `T&&` в шаблоне = универсальная ссылка
- `std::move` = каст, **ничего не двигает**; `std::forward` = сохранение категории; чем отличаются
- Rule of 5: move-конструктор и move-присваивание руками (на `String`/`vector`), `noexcept` на move — **почему без него `vector` копирует вместо двигания** (это условие «реаллокация → relocation, а не copy» из ревью вакансий)
- moved-from состояние: контракт, самопроверка, `swap`-идиома
- `= default` / `= delete`, когда компилятор сам генерирует move (правило: свой деструктор/копирование убивает move)
- чиним `push_back`/`emplace_back` из 13-й лекции: `traits::construct(alloc, newarr + i, std::move(arr[i]))`, perfect forwarding, «почему `emplace_back(const Args&...)` — неправильно»
- практика: `vacancy_tasks/task4_batch` (`SimpleTrajectory` + `TrajectoryBatch`, move-семантика), прогон с `-DCOUNTER` на копиях
- вопросы: «`std::move(x)` — что делает?», «почему `noexcept` на move?», «вернуть by value vs `const&`»

**Лекция 15. Умные указатели и RAII**
- RAII как ответ на утечку из конструктора (11-я лекция) — класс-владелец
- `auto_ptr` (история, почему удалён в C++11), `unique_ptr`: deleter, массивы, невозможность копирования, `std::make_unique`
- `shared_ptr`: контрольный блок, атомарный счётчик ссылок (прямая отсылка к «проблеме подсчёта ссылок» из 13-й), `use_count`, `make_shared` (1 аллокация против 2), `allocate_shared` (связка с 12-й лекцией), custom deleter (SDK-ручки, `FILE*`)
- цикл ссылок → `weak_ptr`, `lock()`/`expired()`, `enable_shared_from_this`
- «Почему frame — `shared_ptr`, а кэш — `unique_ptr`?» (вопрос ревьюера), `vector<unique_ptr<Base>>` (отсылка к 10-й лекции: хранение полиморфных объектов)
- правила: сырой указатель = наблюдение **без владения**; никогда не `shared_ptr<T>(new T)` в аргументах; `shared_from_this()` — UB, пока объект не владеет `shared_ptr`
- практика (домашнее задание): написать свои `scoped_ptr`, `intrusive_ptr`/`shared_ptr` (счётчик в объекте), покрыть тестами, ASan
- бонус: `std::optional`, `std::variant` как «владение без new»

**Лекция 16. Контейнеры, часть 2: свои контейнеры и «внутренности»**
- дописать `vector` целиком: ctor/dtor/`=`, `reserve`, `shrink_to_fit`, `insert`/`erase`, `swap`, strong guarantee (связка с гарантиями из 11-й)
- `list`/`forward_list`: узлы, `rebind` из 12-й, конструктор копирования узловой структуры («глубокая копия» из 13-й), `StackAllocator` — выполнение задания лекции 13
- `deque`: блоки, почему `insert` в середину «дешевле», инвариант
- `set`/`map`/`multimap`: красное-чёрное дерево, инварианты, `operator[]`, `emplace_hint`, почему `map` = 2 аллокации/вставка
- `unordered_set`/`map`: хеши, buckets, `max_load_factor`, худший случай O(n) (вопрос ревьюера: «худший случай `unordered_set` и как застраховаться — reserve, max_load_factor»)
- инвалидация итераторов по каждому контейнеру; complexity-таблица (по стандарту)
- практика: сравнить свой `vector` со `std::vector` на 10⁶ `push_back`, свой пул-аллокатор (задание из 12-й)

### Блок B. STL как единый механизм

**Лекция 17. Итераторы**
- закрыть вопрос из 9-й лекции: `C<T>::type` → `typename` + iterator traits
- 5 категорий итераторов и что они разрешают; `iterator_traits`, `distance`/`advance`
- адаптеры: `reverse_iterator`, `back_inserter`/`front_inserter`/`inserter`, `const_iterator` vs `iterator`
- написать свой итератор для своего `vector`/`list` (+ `begin`/`end`, `cbegin`, C++17 `data()`)
- указатель как итератор; почему `vector` даёт random access; `string_view`/`span` (C++17/20)

**Лекция 18. Алгоритмы + функторы + `std::function`**
- `<algorithm>`: `find/copy/transform/sort/lower_bound/binary_search/accumulate/reduce/for_each/partition/remove_if` (и «`remove` не удаляет»)
- бинарный поиск на «функции стоимости» (то, что уже сделано в `practice/trajectory_perf`)
- функторы (из 10-й) vs указатель на функцию vs лямбда: инлайнинг, размер, тип
- `std::function`: type erasure, цена (аллокация, непрямой вызов), когда не надо
- `bind`, `mem_fn`, проекции/компараторы

**Лекция 19. Лямбды**
- тип замыкания (анонимный класс с `operator()`), это буквально функтор из 6-й/10-й лекции
- захваты: по значению/по ссылке, `[this]`, `[=]`/`[&]` ловушки (висячие ссылки), init-захват C++14, `[p = std::move(p)]`
- `mutable`, рекурсия через `std::function` / Y-combinator
- лямбды в шаблонах: вывод типа компаратора, «уникальный тип → инлайнинг → быстрее `qsort`» (уже упоминалось в 10-й)

### Блок C. Шаблоны: продвинутый уровень

**Лекция 20. Template metaprogramming «на земле»**
- SFINAE и `std::enable_if` (C++11), detection idiom
- tag dispatch, `if constexpr` (C++17) — закрыть «грамотный способ вместо `typeid`» из 8-й лекции
- CRTP довести до конца (из 10-й): static polymorphism, счётчик объектов, неинтерфейсные базы
- policy-based design; partial specialization для функций запрещена → приёмы
- deduction guides и CTAD (C++17), `constexpr` функции, `constinit`
- Concepts (C++20) — обзор: чем заменяют `enable_if`
- практика: реализовать `pair`, свой `tuple` с `get<I>` и `get<T>` (прямое задание из 10-й лекции!)

### Блок D. Многопоточность (уже начато в `multithreaded_programming/`)

**Лекция 21. Потоки, гонки, мьютексы**
- `std::thread`: ctor с аргументами (move-only → `std::unique_ptr`/`std::ref`), `join`/`detach`, «деструктор потока убивает процесс» → RAII `joiner` (снова RAII)
- data race, UB; `std::mutex`, `lock_guard`/`unique_lock`/`scoped_lock` (C++17), `std::lock` от дедлока, `std::recursive_mutex`, reader-writer
- `condition_variable`: `wait` + предикат, lost wakeup, spurious wakeup, «кто владеет мьютексом во время `wait`»
- `std::atomic`: операции, `fetch_add`, memory order (`relaxed/acquire/release/seq_cst`), почему `atomic<shared_ptr>` — плохая идея
- практика: починить `test.cpp` (там `thread_safe_stack`, `atomic<bool> end_of_data`) — заставить компилироваться и быть корректным

**Лекция 22. Задачи и параллельные алгоритмы**
- `std::async` (launch policy), `std::future`/`std::shared_future`, `std::promise`, `std::packaged_task`
- разбор `parallel_quick_sort` из `multithreaded_programming/test.cpp` (это буквально Anthony Williams)
- thread-safe stack/queue, producer-consumer, thread pool, work stealing
- `hardware_concurrency`, балансировка, скалярная деградация; TSan

### Блок E. Производительность, инструменты, проект

**Лекция 23. Производительность и модель памяти**
- кэш, false sharing, выравнивание, SSO, «копия vector в горячем пути = секунды» (из `vacancy_tasks/README.md`)
- профилирование: -O2, `-fno-exceptions`, бенчмарки, перф-бюджет как в задачах `vacancy_tasks`
- инструменты: ASan/UBSan/TSan, valgrind, gdb, perf; `-Wall -Wextra`
- числовые методы (`practice/numerical_recipes`) и `practice/distribution` — как тесты на аллокаторы/кэш

**Лекция 24. Инженерная практика: сборка, тесты, CI**
- CMake (в репо уже `.github/workflows/cmake-multi-platform.yml`), заголовки/`.cpp`, ODR, PImpl, компиляция
- unit-тесты (Catch2/GoogleTest), «тест ДО фикса» — красный → зелёный (формат `vacancy_tasks/`)
- git-гигиена, code review, стиль (Yandex C++ style), документация

**Лекция 25. Итоговый мок-собес**
- 2 часа: ООП-иерархия + vtable → шаблоны → исключения → аллокаторы/контейнеры → move → умные указатели → потоки
- разбор типовых вопросов (`questions.md` по каждой лекции — уже есть готовая база)
- разбор собственных реализаций: свой `vector`, свой `shared_ptr`, свой `tuple` — это портфолио

---

## 4. Порядок и обоснование зависимостей

```
13 (контейнеры, аллокаторы)
   └─ 14 (move)  ──────────────┐   ← без move нельзя починить push_back/emplace_back
        └─ 15 (умные указатели)│   ← rule of 5 + noexcept-движение = shared_ptr/unique_ptr
             ├─ 16 (свои контейнеры: list/deque/map/unordered)
             ├─ 17 (итераторы) → 18 (алгоритмы/функторы) → 19 (лямбды)
             └─ 20 (SFINAE/CRTP/if constexpr)
                  └─ 21 (потоки) → 22 (futures/пулы)   ← опирается на RAII(11), unique_ptr(15), atomics
                        └─ 23 (перф) → 24 (CI/тесты) → 25 (мок)
```

Ключевые аргументы по порядку:
1. **Move до умных указателей**: rule of 5, `noexcept`, «moved-from контракт» — это фундамент `unique_ptr`/`shared_ptr`.
2. **Свои контейнеры (16) сразу после move**, потому что 13-я лекция прямо требует «напишите `list` с нестандартным аллокатором».
3. **Итераторы до алгоритмов** — вопрос «что такое итераторы?» задан на 9-й и не закрыт.
4. **Многопоточность после RAII + умных указателей**: весь код в `multithreaded_programming/test.cpp` держит `shared_ptr`/`unique_ptr`, move, promise/future — раньше 15-й он непонятен.

---

## 5. Хвосты, которые стоит подтянуть параллельно

1. **Лекции 1–4**: `readme.md` пустые (0 байт), темы есть только в комментариях `test.cpp` (введение, модификаторы типов, указатели, ссылки, введение в ООП). Проще всего восстановить как `final_readme.md` в том же формате — на них прямо ссылается 5-я («на прошлой лекции 2а/2б/2в»).
2. **Лекция 13**: нет `final_readme.md` и `questions.md` — единственная из «взрослых» лекций без чистовика. Закрыть до перехода к 14-й (темы: 2 группы контейнеров, complexity-таблица, `push_back` на бумажке, 2 экзаменационных вопроса).
3. **Задание «реализуйте простейший аналог `tuple`»** (из 10-й) — не сделано; включить как ДЗ к 20-й.
4. **Задание «реализуйте `less`/компаратор»** (из 10-й) — к 18-й.
5. **`vacancy_tasks/task4_batch`** — лучшее домашнее задание к 14-й, уже готов (тесты + reference).
6. Формат ДЗ сохранить как в `practice/` и `vacancy_tasks/`: сначала тест (включая перф-бюджет) → красное → фикс → зелёное, обязательно `-fsanitize=address`.

---

## 6. Литература под план (всё лежит в `literature/`)

| Лекции | Книга |
|---|---|
| 14–15 | **Effective and Modern C++** (Мейерс) — items про rvalue-ссылки, `move`, `unique_ptr`/`shared_ptr`/`weak_ptr`, `make_*`; **Effective C++** Мейерса (правила 3/4/13/14/20–22) |
| 16–19 | **C++17 STL. Стандартная библиотека шаблонов (2018)** — контейнеры, итераторы, алгоритмы, `optional`/`variant`/`string_view`/`any` |
| 20 | Effective Modern C++ (traits, `enable_if`, `constexpr`), стандарт (N4860) для цитат про complexity |
| 21–22 | **Параллельное программирование на C++** (Энтони Уильямс) — прямо соответствует коду в `multithreaded_programming/test.cpp` |
| 23–24 | **C++: Библия для пользователя** (общие разделы, отладка), стандарт для формулировок «as if», guarantees |

---

## 7. Если читать сжато (минимум, чтобы не «провалиться» на план после 13-й)

14 → move semantics (lvalue/rvalue, `std::move`/`forward`, rule of 5, `noexcept`)
15 → smart pointers (`unique_ptr`, `shared_ptr`, `weak_ptr`)
16 → свои контейнеры (list/deque/map/unordered) + аллокаторы из 12-й
17 → итераторы
18 → алгоритмы + функторы + `std::function`
19 → лямбды
20 → SFINAE/CRTP/`if constexpr`
21–22 → потоки (`thread`, `mutex`, `condition_variable`, `atomic`, `future`)
23 → производительность + санитайзеры
24 → CMake/тесты/CI
25 → мок-собес
