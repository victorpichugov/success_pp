// Тесты SimpleTrajectory + TrajectoryBatch (ТЗ — README.md).
//
// ВАЖНО: скелет batch.h НЕ КОМПИЛИРУЕТСЯ — это ожидаемо
// (copy = delete, move отсутствует: vector<TrajectoryBatch>,
// ExtractAll, push_back(std::move(...)) — всё падает на этапе
// компиляции). Это и есть «красное» до фикса.
//
// Сборка: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o t && ./t
//
// Структура: счётчик failures, каждый тест в своём {}-блоке,
// return failures. Минимальный набор (см. README, «Тесты»):
//   a) static_assert (noexcept move, !copy);
//   b) move-конструктор SimpleTrajectory (Empty/Size/frame/use_count);
//   c) move-присваивание + self-assignment;
//   d) batch: Add x3, ExtractAll, повторный ExtractAll;
//   e) vector<TrajectoryBatch>, push_back(std::move) x 1000;
//   f) PERF: move батча 10^6 точек — бюджет 10 мс.
#include "batch.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

int main() {
  int failures = 0;

  // ТВОИ ТЕСТЫ ЗДЕСЬ

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
