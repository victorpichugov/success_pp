# Task 4. TrajectoryBatch — move между модулями (IPC)

## Контекст (что это в команде)

Модуль прогнозирования (producer) считает партии траекторий окружения
и передаёт их планировщику (consumer) через IPC (общая память /
socket). Партия: 1000 траекторий x 1000 точек x 24 байта = 24 МБ.
Скопировать 24 МБ на каждый кадр — десятки мс чистых memcpy.
Требование: данные ПЕРЕНОСЯТСЯ (move), а не копируются.

Это мини-версия `practice/trajectory_perf` (там — кинематика и кэш;
здесь — только владение и перенос), чтобы отработать паттерны в
изолированном виде.

## ТЗ

`batch.h`:

1. `SimpleTrajectory`:
   - уже объявлено: копирование `= delete` (траектория тяжёлая);
   - **нет** move-операций — `vector<SimpleTrajectory>` не компилируется.
   - Дописать: move-конструктор и move-присваивание, `noexcept`.
     Исходник после move — в валидном состоянии: `Empty() == true`.

2. `TrajectoryBatch`:
   - владение: `frame_` (общая система координат, read-only) +
     `trajs_` (эксклюзивная партия);
   - `Add(SimpleTrajectory traj)` — by value + move (забирает
     владение; отсюда можно передать и lvalue, и rvalue, максимум
     одно перемещение);
   - `ExtractAll()` — consumer забирает ВСЁ: возвращает
     `std::vector<SimpleTrajectory>` по move, батч пустеет;
   - batch тоже move-only (копии `= delete`), move-операции noexcept.

## Устные вопросы (ДО кода — я спрашиваю)

1. Почему компилятор сам move НЕ генерирует, если копирование
   объявлено пользователем (даже `= delete`)? (правило: user-declared
   copy ops suppress move.)
2. Что при move происходит с `frame_` (shared_ptr), `points_` (vector),
   и (если добавить кэш) с `unique_ptr` — по-разному. Разберите каждое.
3. Почему `noexcept` обязателен? (подсказка: реаллокация
   `vector<SimpleTrajectory>`: без noexcept — переезд копией,
   а копирование delete'd -> ошибка компиляции / или медленный copy
   при его наличии.)
4. `Add(SimpleTrajectory)` — by value: сколько копий/move при вызове
   `batch.Add(std::move(t))`? А `batch.Add(t)` (lvalue)? Почему не
   `const&` (тут владение) и не `T&&` (потеря lvalue-варианта)?
5. `ExtractAll()` — by value: что вернёт consumer после того, как
   батч передали `std::move(batch)` в vector? Почему батч должен
   пустеть, а не «раздавать ссылки»?

## Тесты (минимальный набор, в main.cpp)

a) static_assert:
   - `std::is_nothrow_move_constructible_v<SimpleTrajectory>`;
   - `std::is_nothrow_move_assignable_v<SimpleTrajectory>`;
   - `!std::is_copy_constructible_v<SimpleTrajectory>`;
   - `std::is_nothrow_move_constructible_v<TrajectoryBatch>`.
b) move-конструктор: `SimpleTrajectory b(std::move(a))` —
   `a.Empty() == true`, `b.Size() == n`, frame переехал
   (`a.Frame() == nullptr`, `b.Frame() == frame.get()`,
   `frame.use_count()` не изменился).
c) move-присваивание: `c = std::move(a)`; self-assignment
   `d = std::move(d)` не падает (guard `this != &other`).
d) batch: `Add` x 3 -> `Size() == 3`; `ExtractAll` -> 3 траектории,
   `Size() == 0`; повторный `ExtractAll` -> пусто.
e) `std::vector<TrajectoryBatch> v; v.push_back(std::move(batch));`
   — компилируется (move у batch есть), 1000 итераций, `v.size() == 1000`.
f) PERF: батч 1000 траекторий x 1000 точек (10^6 точек);
   `TrajectoryBatch b2 = std::move(batch)` — бюджет 10 мс
   (move vector'а — 3 указателя, должно быть микросекунды);
   после: `b2.Size() == 1000`, `batch.Size() == 0`.

## Сдача

«готово» + полный вывод ./t.
