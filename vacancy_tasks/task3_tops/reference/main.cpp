// Эталонные тесты WeightedIntersect. Не запускать до «готово».
#include "weighted_tops_fixed.h"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>

#ifndef PERF_N
#define PERF_N 100000
#endif

namespace {

void Check(const std::string& name, const std::vector<int>& got,
           const std::vector<int>& expect, int& failures) {
  if (got != expect) {
    std::cerr << "FAIL " << name << ": got [";
    for (std::size_t i = 0; i < got.size() && i < 20; ++i) {
      std::cerr << (i ? " " : "") << got[i];
    }
    if (got.size() > 20) std::cerr << " ...";
    std::cerr << "] expect [";
    for (std::size_t i = 0; i < expect.size() && i < 20; ++i) {
      std::cerr << (i ? " " : "") << expect[i];
    }
    if (expect.size() > 20) std::cerr << " ...";
    std::cerr << "]\n";
    ++failures;
  } else {
    std::cout << "  " << name << " OK (size = " << got.size() << ")\n";
  }
}

}  // namespace

int main() {
  int failures = 0;

  // a) точность
  {
    std::vector<std::vector<int>> got;
    got.push_back(WeightedIntersect({5, 3, 9, 7}, {3, 5, 9, 8}));
    Check("a1", got.back(), {5, 3, 9}, failures);

    // Критичный: порядок top_a даёт {5,9,1} — НЕВЕРНО.
    got.push_back(WeightedIntersect({42, 7, 5, 9, 3, 1}, {9, 5, 1, 8, 2, 6}));
    Check("a2", got.back(), {9, 5, 1}, failures);

    got.push_back(WeightedIntersect({10, 1}, {1, 10}));
    Check("a3", got.back(), {10, 1}, failures);

    got.push_back(WeightedIntersect({1, 2, 3}, {3, 2, 1}));
    Check("a4", got.back(), {1, 2, 3}, failures);

    got.push_back(WeightedIntersect({1, 2}, {3, 4}));
    Check("a5", got.back(), {}, failures);

    got.push_back(WeightedIntersect({}, {1, 2}));
    Check("a6", got.back(), {}, failures);

    got.push_back(WeightedIntersect({7}, {7}));
    Check("a7", got.back(), {7}, failures);

    got.push_back(WeightedIntersect({1, 100}, {100, 1}));
    Check("a8", got.back(), {1, 100}, failures);
  }

  // b) perf: n = m = PERF_N, пересечение PERF_N/2.
  {
    const std::size_t n = PERF_N;
    std::vector<int> top_a(n), top_b(n);
    for (std::size_t i = 0; i < n; ++i) {
      top_a[i] = static_cast<int>(i);
      top_b[i] = static_cast<int>(i + n / 2);
    }
    std::vector<int> expect(n / 2);
    for (std::size_t i = 0; i < n / 2; ++i) {
      expect[i] = static_cast<int>(n / 2 + i);
    }

    const auto t0 = std::chrono::steady_clock::now();
    const std::vector<int> got = WeightedIntersect(top_a, top_b);
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "  b: " << ms << " ms (n = m = " << n
              << ", пересечение " << n / 2 << ")\n";

    if (got != expect) {
      std::cerr << "FAIL b: содержимое (size got = " << got.size()
                << ", expect = " << expect.size() << ")\n";
      ++failures;
    }
    if (ms > 300) {
      std::cerr << "FAIL b: бюджет 300 мс (O(n*m)?)\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
