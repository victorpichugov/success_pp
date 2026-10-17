// Тесты WeightedIntersect (ТЗ — README.md).
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//   legacy (посмотреть, что «краснеет» по перфу):
//   g++ -std=c++17 -Wall -Wextra -O2 -DPERF_N=20000 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор:
//   a) точность: 8 кейсов из README (ПОЛНОЕ попарное сравнение векторов);
//   b) perf: n = m = PERF_N (по умолчанию 100 000), top_a = [0..PERF_N-1],
//      top_b = [PERF_N/2 .. 3*PERF_N/2 - 1] -> пересечение PERF_N/2,
//      ожидаемый результат [PERF_N/2 .. PERF_N-1] в этом порядке;
//      бюджет 300 мс, steady_clock.
#include "weighted_tops.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>

#ifndef PERF_N
#define PERF_N 100000
#endif

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
