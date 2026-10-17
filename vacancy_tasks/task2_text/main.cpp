// Тесты CleanLine (ТЗ — README.md).
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор:
//   a) точность: 13 кейсов из README (строка И оба счётчика — точное
//      сравнение; порядок символов важен);
//   b) perf: строка 100 000 символов, ~50 000 смайликов (паттерн
//      "ab :-) c " x N), бюджет 100 мс, steady_clock.
//      На legacy — «красный» (секунды, O(n^2) по substr).
#include "cleaner.h"

#include <chrono>
#include <iostream>
#include <string>

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
