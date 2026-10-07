// Тесты DiscreteDistribution (ТЗ п.6), без фреймворка.
//
// Структура:
//   - int failures — счётчик упавших проверок;
//   - каждый тест в своём {}-блоке (изоляция переменных);
//   - return failures — код выхода: 0 = всё OK.

#include "discrete_distribution.h"

#include <cmath>
#include <iostream>
#include <random>
#include <vector>

int main() {
    int failures = 0;

    // ================================================================
    // a) Детерминизм: одинаковый seed => одинаковая последовательность
    // ================================================================
    {
        std::mt19937_64 r1{7};
        std::mt19937_64 r2{7};
        std::mt19937_64 r3{8};   // другой seed — для контроля

        const DiscreteDistribution d1({1, 2, 3}, {1.0, 3.0, 5.0});
        const DiscreteDistribution d2({1, 2, 3}, {1.0, 3.0, 5.0});

        const int N = 1000;
        bool same = true;
        bool different = false;
        for (int i = 0; i < N; ++i) {
            const int x1 = d1.Sample(r1);
            const int x2 = d2.Sample(r2);
            const int x3 = d2.Sample(r3);
            if (x1 != x2) same = false;
            if (x1 != x3) different = true;
        }
        if (!same) {
            std::cerr << "FAIL a: одинаковые seed'ы дали разные последовательности\n";
            ++failures;
        }
        if (!different) {
            std::cerr << "FAIL a: разные seed'ы дали ОДИНАКОВЫЕ последовательности\n";
            ++failures;
        }
        if (same && different) std::cout << "  a: детерминизм OK\n";
    }

    // ================================================================
    // b) Частоты: weights {1,3,5} -> p = {1/9, 3/9, 5/9}
    // ================================================================
    {
        std::mt19937_64 rng{42};
        const DiscreteDistribution d({1, 2, 3}, {1.0, 3.0, 5.0});

        const int N = 1'000'000;
        std::vector<long> cnt(3, 0);   // cnt[i] — сколько раз выпало values[i]

        for (int i = 0; i < N; ++i) {
            const int x = d.Sample(rng);
            if (x < 1 || x > 3) {     // страховка: вне диапазона = наш баг
                std::cerr << "FAIL b: сэмпл вернул " << x << " (вне {1,2,3})\n";
                ++failures;
                break;
            }
            ++cnt[x - 1];             // values = {1,2,3} => индекс = x - 1
        }

        const double expect[3] = {1.0 / 9.0, 3.0 / 9.0, 5.0 / 9.0};
        for (int i = 0; i < 3; ++i) {
            const double freq = static_cast<double>(cnt[i]) / N;
            if (std::abs(freq - expect[i]) > 0.02) {
                std::cerr << "FAIL b: freq[" << i << "] = " << freq
                    << ", ожидалось ~" << expect[i] << '\n';
                ++failures;
            } else {
                std::cout << "  b: freq[" << i << "] = " << freq
                    << " (ожидалось ~" << expect[i] << ") OK\n";
            }
        }
    }

    // ================================================================
    // c) Валидация: все три ошибки из ТЗ бросают std::invalid_argument
    // ================================================================
    {
        // c1: пустые values
        bool thrown = false;
        try {
            DiscreteDistribution d({}, {1.0});
        } catch (const std::invalid_argument& e) {
            thrown = true;
            std::cout << "  c1: бросило: " << e.what() << '\n';
        }
        if (!thrown) { std::cerr << "FAIL c1: пустые values не бросили\n"; ++failures; }

        // c2: разные размеры
        thrown = false;
        try {
            DiscreteDistribution d({1, 2}, {1.0});
        } catch (const std::invalid_argument& e) {
            thrown = true;
            std::cout << "  c2: бросило: " << e.what() << '\n';
        }
        if (!thrown) { std::cerr << "FAIL c2: разнобой размеров не бросил\n"; ++failures; }

        // c3: НЕГАТИВНЫЙ ВЕС В ПЕРВОМ ЭЛЕМЕНТЕ.
        // ВАЖНО: в вашем заголовке цикл валидации идёт с i = 1 —
        // этот тест должен ПАДТЬ, пока вы это не поправите.
        // Это демонстрация того, зачем тесты вообще существуют.
        thrown = false;
        try {
            DiscreteDistribution d({1, 2}, {-1.0, 5.0});
        } catch (const std::invalid_argument& e) {
            thrown = true;
            std::cout << "  c3: бросило: " << e.what() << '\n';
        }
        if (!thrown) { std::cerr << "FAIL c3: weights[0] <= 0 не пойман\n"; ++failures; }
    }

    // ================================================================
    // d) Границы
    // ================================================================
    {
        // d1: одно значение — всегда оно
        {
            std::mt19937_64 rng{1};
            const DiscreteDistribution d({42}, {7.0});
            bool all = true;
            for (int i = 0; i < 1000; ++i) {
                if (d.Sample(rng) != 42) all = false;
            }
            if (!all) { std::cerr << "FAIL d1: единственное значение не всегда\n"; ++failures; }
            else std::cout << "  d1: one-value OK\n";
        }

        // d2: огромные веса.
        // Вопрос, на который должен ответить ВАШ вывод (прочитайте его!):
        // что такое 1e16 + 1.0 в double? Сколько значащих цифр у double?
        // И что это делает со вторым сегментом на «числовой прямой»?
        const double sum = 1e16 + 1.0;
        std::cout << "  d2: 1e16 + 1.0 = " << sum << '\n';

        std::mt19937_64 rng{3};
        const DiscreteDistribution d({1, 2}, {1e16, 1.0});
        long cnt2 = 0;
        const int N = 1'000'000;
        for (int i = 0; i < N; ++i) {
            if (d.Sample(rng) == 2) ++cnt2;
        }
        std::cout << "  d2: значение 2 выпало " << cnt2 << " раз из " << N
            << " (идеальная вероятность 1e-16 => ~0)\n";
        // ВАШ ЗАКЛЮЧАЮЩИЙ ВЫВОД (устно мне):
        // cdf[1] == cdf[0]? Откуда этот факт следует из вывода выше?
        // Значит ли это, что код "сломан" — или это ограничение double?
    }

    // ================================================================
    // e) Move: moved-to живёт, moved-from — по задокументированному контракту
    // ================================================================
    {
        DiscreteDistribution src({1, 2, 3}, {1.0, 3.0, 5.0});
        DiscreteDistribution dst = std::move(src);

        std::mt19937_64 rng{5};
        const int x = dst.Sample(rng);
        if (x < 1 || x > 3) {
            std::cerr << "FAIL e: moved-to объект сломан\n";
            ++failures;
        } else {
            std::cout << "  e: moved-to сэмплирует OK (x = " << x << ")\n";
        }

        // moved-from: cdf_ пустой, Sample() должен бросить logic_error
        // (защита, добавленная в Sample() — не UB, а управляемый сбой).
        bool movedFromThrows = false;
        try {
            src.Sample(rng);
        } catch (const std::logic_error& e) {
            movedFromThrows = true;
            std::cout << "  e: moved-from бросает logic_error: "
                << e.what() << '\n';
        }
        if (!movedFromThrows) {
            std::cerr << "FAIL e: moved-from не бросил logic_error\n";
            ++failures;
        }
    }

    std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
        << ": " << failures << " failure(s)\n";
    return failures;
}