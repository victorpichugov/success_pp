// Тесты AliasedSampler (ТЗ — README.md, «Тесты»).
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор:
//   a) детерминизм (seed 7 vs 7 -> одинаково; 7 vs 8 -> разное);
//   b) частоты {1,3,5} -> {1/9, 3/9, 5/9}, N = 1e6, допуск 0.015;
//   c) одно значение;
//   d) валидация (7 подкейсов -> std::invalid_argument);
//   e) move (dst валиден; src -> std::logic_error);
//   f) PERF: n = 1000, N = 1e7, бюджет 300 мс;
//   g) огромные веса {1e12, 1.0}: напечатать частоты.
#include "sampler.h"

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
