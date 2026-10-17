#pragma once

#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>

// Подход (A) из README: хеш-таблица позиций top_b + сканирование top_a
// + сортировка пересечения.
//
// Time: в среднем O(n + m) (заполнение map + сканирование) +
//       O(k log k) (сортировка пересечения), k = |пересечение|.
// Space: O(m) на map + O(k) на промежуточный вектор.
//
// Защита от дегенерации unordered_map: reserve + max_load_factor ДО
// заполнения (иначе реаллокации + рост коллизий).
//
// Спецификация: в каждом списке id уникальны; результат — id из обоих
// списков, отсортированные по (min(pos_a, pos_b), pos_a) по возрастанию.
std::vector<int> WeightedIntersect(const std::vector<int>& top_a,
                                   const std::vector<int>& top_b) {
  // Кладём в map БОЛЕЕ КОМПАКТНЫЙ список? Нет: порядок результата —
  // из top_a, поэтому map строим по top_b (m), сканируем top_a (n).
  // Если m >> n, имеет смысл развернуть роли и тир-брейк по pos_b —
  // но спека фиксирует тир-брейк по top_a, разворот недопустим.
  std::unordered_map<int, std::size_t> pos_b;
  // max_load_factor ДО reserve: reserve считает кол-во корзин
  // по текущему load factor.
  pos_b.max_load_factor(0.7);
  pos_b.reserve(top_b.size());  // ~1.43*top_b корзин -> без реаллокаций
  for (std::size_t i = 0; i < top_b.size(); ++i) {
    pos_b.emplace(top_b[i], i);
  }

  struct Item {
    std::size_t key;    // min(pos_a, pos_b)
    std::size_t pos_a;  // тир-брейк
    int id;
  };
  std::vector<Item> items;
  items.reserve(std::min(top_a.size(), top_b.size()));
  for (std::size_t i = 0; i < top_a.size(); ++i) {
    const auto it = pos_b.find(top_a[i]);
    if (it != pos_b.end()) {
      items.push_back({std::min(i, it->second), i, top_a[i]});
    }
  }

  // pos_a уникален в items (каждый id из top_a — один раз), так что
  // порядок тотальный; stable_sort не нужен.
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
