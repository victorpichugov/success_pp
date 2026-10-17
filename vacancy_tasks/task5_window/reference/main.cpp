// Эталонные тесты MaxConcurrent. Не запускать до «готово».
#include "traffic_fixed.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

#ifndef PERF_N
#define PERF_N 1000000
#endif

namespace {

// Brute force O(n * |times|): максимум по всем «событным» моментам.
long long BruteForce(const std::vector<long long>& enter,
                     const std::vector<long long>& exit) {
  long long result = 0;
  for (long long t : enter) {  // максимум достигается после въезда
    long long cnt = 0;
    for (std::size_t j = 0; j < enter.size(); ++j) {
      if (enter[j] <= t && exit[j] > t) {
        ++cnt;
      }
    }
    result = std::max(result, cnt);
  }
  return result;
}

}  // namespace

int main() {
  int failures = 0;

  // a) точность
  {
    struct Case {
      std::vector<long long> enter;
      std::vector<long long> exit;
      long long expect;
    };
    const std::vector<Case> cases = {
        {{}, {}, 0},
        {{1, 1, 1}, {2, 2, 2}, 3},
        {{1, 2, 3}, {2, 3, 4}, 1},
        {{5}, {5}, 0},
        {{1, 2}, {3, 3}, 2},
        {{1, 3, 7}, {4, 5, 8}, 2},
        {{0, 0, 1, 1}, {1, 2, 2, 3}, 3},
    };
    for (std::size_t i = 0; i < cases.size(); ++i) {
      const long long got =
          MaxConcurrent(cases[i].enter, cases[i].exit);
      if (got != cases[i].expect) {
        std::cerr << "FAIL a" << i << ": got " << got
                  << ", expect " << cases[i].expect << "\n";
        ++failures;
      }
    }
    if (failures == 0) {
      std::cout << "  a: " << cases.size() << " точность кейсов OK\n";
    }
  }

  // b) fuzz: 300 случайных малых случаев против brute force
  {
    std::mt19937 rng{12345};
    int fuzz_fail = 0;
    for (int iter = 0; iter < 300; ++iter) {
      const int n = static_cast<int>(rng()) % 41;  // 0..40
      std::vector<long long> enter(n), exit(n);
      for (int i = 0; i < n; ++i) {
        const long long e = static_cast<long long>(rng() % 51);  // 0..50
        const long long d = static_cast<long long>(rng() % 31);  // 0..30
        enter[i] = e;
        exit[i] = e + d;
      }
      std::sort(enter.begin(), enter.end());
      std::sort(exit.begin(), exit.end());
      const long long got = MaxConcurrent(enter, exit);
      const long long expect = BruteForce(enter, exit);
      if (got != expect) {
        if (fuzz_fail < 5) {
          std::cerr << "FAIL b: fuzz #" << iter << " got " << got
                    << ", expect " << expect << " (n = " << n << ")\n";
        }
        ++fuzz_fail;
      }
    }
    if (fuzz_fail > 0) {
      std::cerr << "FAIL b: fuzz — упало " << fuzz_fail << " из 300\n";
      ++failures;
    } else {
      std::cout << "  b: fuzz 300/300 OK\n";
    }
  }

  // c) perf: n = PERF_N, enter[i] = 2*i, exit[i] = 2*i + PERF_N/2.
  {
    const std::size_t n = PERF_N;
    std::vector<long long> enter(n), exit(n);
    for (std::size_t i = 0; i < n; ++i) {
      enter[i] = 2 * static_cast<long long>(i);
      exit[i] = 2 * static_cast<long long>(i) + static_cast<long long>(n / 2);
    }
    const long long expect = static_cast<long long>(n / 2);
    const auto t0 = std::chrono::steady_clock::now();
    const long long got = MaxConcurrent(enter, exit);
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "  c: " << ms << " ms (n = " << n << ", got = " << got
              << ", expect " << expect << ")\n";
    if (got != expect) {
      std::cerr << "FAIL c: содержимое\n";
      ++failures;
    }
    if (ms > 100) {
      std::cerr << "FAIL c: бюджет 100 мс (O(n^2)?)\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
