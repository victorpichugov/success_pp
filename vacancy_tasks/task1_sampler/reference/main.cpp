// Эталонные тесты AliasedSampler. Не запускать до «готово».
#include "sampler_alias.h"

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace {

bool ThrowsInvalidArgument(const std::function<void()>& f) {
  try {
    f();
    return false;
  } catch (const std::invalid_argument&) {
    return true;
  } catch (...) {
    return false;  // другой тип исключения = провал
  }
}

}  // namespace

int main() {
  int failures = 0;

  // a) детерминизм
  {
    AliasedSampler s1({1, 2, 3}, {1.0, 3.0, 5.0});
    AliasedSampler s2({1, 2, 3}, {1.0, 3.0, 5.0});
    std::mt19937_64 r1{7};
    std::mt19937_64 r2{7};
    std::mt19937_64 r3{8};
    bool same = true;
    bool different = false;
    for (int i = 0; i < 1000; ++i) {
      const int x1 = s1.Sample(r1);
      const int x2 = s2.Sample(r2);
      const int x3 = s2.Sample(r3);
      if (x1 != x2) same = false;
      if (x1 != x3) different = true;
    }
    if (!same || !different) {
      std::cerr << "FAIL a: детерминизм (same=" << same
                << ", different=" << different << ")\n";
      ++failures;
    } else {
      std::cout << "  a: детерминизм OK\n";
    }
  }

  // b) частоты
  {
    AliasedSampler s({1, 2, 3}, {1.0, 3.0, 5.0});
    std::mt19937_64 rng{42};
    const int N = 1'000'000;
    std::vector<long> cnt(3, 0);
    for (int i = 0; i < N; ++i) {
      const int x = s.Sample(rng);
      if (x < 1 || x > 3) {
        std::cerr << "FAIL b: сэмпл " << x << " вне {1,2,3}\n";
        ++failures;
        break;
      }
      ++cnt[x - 1];
    }
    const double expect[3] = {1.0 / 9.0, 3.0 / 9.0, 5.0 / 9.0};
    bool ok = true;
    for (int i = 0; i < 3; ++i) {
      const double freq = static_cast<double>(cnt[i]) / N;
      if (std::abs(freq - expect[i]) > 0.015) ok = false;
      std::cout << "  b: freq[" << i << "] = " << freq
                << " (ожидалось ~" << expect[i] << ")\n";
    }
    if (!ok) {
      std::cerr << "FAIL b: частоты\n";
      ++failures;
    }
  }

  // c) одно значение
  {
    AliasedSampler s({42}, {7.0});
    std::mt19937_64 rng{1};
    bool all = true;
    for (int i = 0; i < 1000; ++i) {
      if (s.Sample(rng) != 42) all = false;
    }
    if (!all) {
      std::cerr << "FAIL c: единственное значение\n";
      ++failures;
    } else {
      std::cout << "  c: one-value OK\n";
    }
  }

  // d) валидация
  {
    const bool ok =
        ThrowsInvalidArgument([] { AliasedSampler s({}, {1.0}); }) &&
        ThrowsInvalidArgument([] { AliasedSampler s({1}, {}); }) &&
        ThrowsInvalidArgument([] { AliasedSampler s({1, 2}, {1.0}); }) &&
        ThrowsInvalidArgument([] { AliasedSampler s({1, 2}, {0.0, 1.0}); }) &&
        ThrowsInvalidArgument([] { AliasedSampler s({1, 2}, {-1.0, 2.0}); }) &&
        ThrowsInvalidArgument(
            [] { AliasedSampler s({1, 2}, {std::nan(""), 1.0}); }) &&
        ThrowsInvalidArgument([] {
          AliasedSampler s(
              {1, 2}, {1.0, std::numeric_limits<double>::infinity()});
        });
    if (!ok) {
      std::cerr << "FAIL d: валидация\n";
      ++failures;
    } else {
      std::cout << "  d: валидация OK (7 подкейсов)\n";
    }
  }

  // e) move
  {
    AliasedSampler src({1, 2, 3}, {1.0, 3.0, 5.0});
    AliasedSampler dst = std::move(src);
    std::mt19937_64 rng{5};
    bool ok = true;
    for (int i = 0; i < 1000; ++i) {
      const int x = dst.Sample(rng);
      if (x < 1 || x > 3) ok = false;
    }
    bool movedFromThrows = false;
    try {
      src.Sample(rng);
    } catch (const std::logic_error&) {
      movedFromThrows = true;
    }
    if (!ok || !movedFromThrows) {
      std::cerr << "FAIL e: move (ok=" << ok
                << ", movedFromThrows=" << movedFromThrows << ")\n";
      ++failures;
    } else {
      std::cout << "  e: move OK (moved-from бросает logic_error)\n";
    }
  }

  // f) perf: n = 1000, N = 1e7
  {
    const int n = 1000;
    std::vector<int> values(n);
    std::vector<double> weights(n, 1.0);
    for (int i = 0; i < n; ++i) {
      values[i] = i;
    }
    AliasedSampler s(std::move(values), std::move(weights));
    std::mt19937_64 rng{99};
    const int N = 10'000'000;
    long sum = 0;  // anti dead-code elimination
    const auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
      sum += s.Sample(rng);
    }
    const auto t1 = std::chrono::steady_clock::now();
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "  f: 1e7 сэмплиров: " << ms << " ms (сумма = " << sum
              << ")\n";
    if (ms > 300) {
      std::cerr << "FAIL f: бюджет 300 мс\n";
      ++failures;
    }
  }

  // g) огромные веса: {1e12, 1.0}.
  // Сумма = 1e12 + 1.0 точно представима (spacing на уровне 1e12 — 2^-12).
  // p(2) = 1e-12 -> из 1e6 сэмплов ~0–2 выпадения.
  // CDF-подход: cdf = [1e12, 1e12 + 1] — здесь ещё различим, но при
  // 1e16: cdf[1] == cdf[0] и второе значение НЕВЫПАДАЕТ никогда.
  {
    AliasedSampler s({1, 2}, {1e12, 1.0});
    std::mt19937_64 rng{3};
    const int N = 1'000'000;
    long cnt2 = 0;
    for (int i = 0; i < N; ++i) {
      if (s.Sample(rng) == 2) ++cnt2;
    }
    std::cout << "  g: значение 2 выпало " << cnt2 << " раз из " << N
              << " (теоретически ~0–2)\n";
    if (cnt2 > 20) {
      std::cerr << "FAIL g: подозрительно много " << cnt2 << "\n";
      ++failures;
    }
  }

  std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
            << ": " << failures << " failure(s)\n";
  return failures;
}
