// Тесты ParseLog (ТЗ — README.md).
//
// Legacy «красный»: каноничные строки — зелёные, всё остальное (порядок,
// доп. поля, "1.5.5", "nan", пустые значения) — красное.
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор:
//   a) точность: 13 кейсов из README (события ТОЧНО + skipped ТОЧНО);
//   b) смешанный лог: 3 строки (2 OK + 1 битая) -> 2 events, 1 skipped;
//   c) perf: 100 000 валидных строк, бюджет 200 мс.
#include "log_parser.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
