// Тесты MaxConcurrent (ТЗ — README.md).
//
// ВАЖНО: legacy на МАЛОМ n «зелёный» по содержимому — «красное» здесь
// перфу и, возможно, fuzz (если legacy считает неправильно).
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//   legacy с перфом:
//   g++ -std=c++17 -Wall -Wextra -O2 -DPERF_N=2000 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор:
//   a) точность: 7 кейсов из README (включая enter == exit);
//   b) FUZZ: 300 случайных случаев (n до 40, времена 0..50),
//      сравнение с O(n^2) brute force по ВСЕМ событийным моментам;
//   c) perf: n = PERF_N (по умолчанию 10^6), enter[i] = 2*i,
//      exit[i] = 2*i + PERF_N/2 -> ответ PERF_N/2, бюджет 100 мс.
#include "traffic.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

#ifndef PERF_N
#define PERF_N 1000000
#endif

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
