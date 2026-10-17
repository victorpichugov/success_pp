#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

// Legacy: наивный перебор O(n^2) — для каждого въезда считаем,
// сколько машин на участке в этот момент.
//
// Замечание ревьюера: n = 10^6 -> 10^12 сравнений (часы).
// ТЗ (README.md): O(n) время, O(1) доп. память — два указателя.
//
// Спецификация: enter, exit — ОТСОРТИРОВАННЫЕ, одной длины;
// присутствие: enter <= t < exit; пустой ввод -> 0.
long long MaxConcurrent(const std::vector<long long>& enter,
                        const std::vector<long long>& exit) {
  long long result = 0;
  for (std::size_t i = 0; i < enter.size(); ++i) {
    long long cnt = 0;
    for (std::size_t j = 0; j < enter.size(); ++j) {
      if (enter[j] <= enter[i] && exit[j] > enter[i]) {
        ++cnt;
      }
    }
    result = std::max(result, cnt);
  }
  return result;
}
