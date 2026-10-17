#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

// Legacy. Ранжирование 2022: «просто работает» на топах по 500 id.
//
// Замечание ревьюера: двойной цикл O(n * m). На n = m = 100k это 10^10
// сравнений — минуты. ТЗ: в среднем O(n + m) + O(k log k), см. README.
//
// Спецификация (см. README.md): в каждом списке id уникальны;
// результат — id из обоих списков, отсортированные по
// (min(pos_a, pos_b), pos_a) по возрастанию.
std::vector<int> WeightedIntersect(const std::vector<int>& top_a,
                                   const std::vector<int>& top_b) {
  struct Item {
    std::size_t key;
    std::size_t pos_a;
    int id;
  };
  std::vector<Item> items;
  // Замечание ревьюера: O(n * m)
  for (std::size_t i = 0; i < top_a.size(); ++i) {
    for (std::size_t j = 0; j < top_b.size(); ++j) {
      if (top_a[i] == top_b[j]) {
        items.push_back({std::min(i, j), i, top_a[i]});
      }
    }
  }
  std::sort(items.begin(), items.end(), [](const Item& x, const Item& y) {
    if (x.key != y.key) return x.key < y.key;
    return x.pos_a < y.pos_a;
  });
  std::vector<int> result;
  result.reserve(items.size());
  for (const Item& it : items) {
    result.push_back(it.id);
  }
  return result;
}
