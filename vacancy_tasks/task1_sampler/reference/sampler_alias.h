#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

// Walker's Alias Method (Vose, 1994).
//
// Контракт:
//   - конструктор: O(n), память O(n); бросает std::invalid_argument
//     на невалидных входных данных;
//   - Sample(): O(1) в среднем (два O(1)-распределения + ветвление),
//     выделений в горячем пути НЕТ;
//   - после std::move исходник валиден, но пуст; Sample() на нём
//     бросает std::logic_error.
class AliasedSampler {
 public:
  AliasedSampler(std::vector<int> values, std::vector<double> weights)
      : values_(std::move(values)),
        prob_(values_.size()),
        alias_(values_.size()) {
    if (values_.empty() || weights.empty()) {
      throw std::invalid_argument("values и weights не должны быть пустыми");
    }
    if (values_.size() != weights.size()) {
      throw std::invalid_argument("размеры values и weights должны совпадать");
    }
    for (std::size_t i = 0; i < weights.size(); ++i) {
      if (!std::isfinite(weights[i]) || weights[i] <= 0.0) {
        throw std::invalid_argument(
            "веса должны быть положительными и конечными");
      }
    }
    const double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (!std::isfinite(sum) || sum <= 0.0) {
      throw std::invalid_argument("сумма весов должна быть положительной и конечной");
    }
    // «Доля корзины»: n * p_i, где p_i = w_i / sum. Сумма по i = n.
    for (std::size_t i = 0; i < weights.size(); ++i) {
      prob_[i] = static_cast<double>(values_.size()) * weights[i] / sum;
    }
    BuildAlias();
  }

  // Купе: корзина b равномерно [0, n), монетка u равномерно [0, 1).
  // O(1), выделений нет (distributions — мелкие POD, конструируются
  // за O(1) на каждый вызов).
  int Sample(std::mt19937_64& rng) const {
    if (values_.empty()) {
      throw std::logic_error("Sample() на moved-from / пустом объекте");
    }
    const int n = static_cast<int>(values_.size());
    const int basket = std::uniform_int_distribution<int>(0, n - 1)(rng);
    const double coin = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    return coin < prob_[basket] ? values_[basket] : values_[alias_[basket]];
  }

  std::size_t Size() const { return values_.size(); }

 private:
  // Распределяем «излишек» тяжёлых значений по недоливу лёгких.
  // less — значения с prob_ < 1 (нужен недолив 1 - prob_),
  // heavy — значения с prob_ > 1 (могут отдать излишек).
  // Каждый шаг либо уменьшает |heavy|, либо уменьшает «хвост» less:
  // суммарно не больше n шагов.
  void BuildAlias() {
    const std::size_t n = values_.size();
    for (std::size_t i = 0; i < n; ++i) {
      alias_[i] = static_cast<int>(i);
    }
    std::vector<int> less;
    std::vector<int> heavy;
    less.reserve(n / 2 + 1);
    heavy.reserve(n / 2 + 1);
    for (std::size_t i = 0; i < n; ++i) {
      if (prob_[i] < 1.0) {
        less.push_back(static_cast<int>(i));
      } else if (prob_[i] > 1.0) {
        heavy.push_back(static_cast<int>(i));
      }
    }
    // less — очередь (с фронта), heavy — берём с хвоста.
    std::size_t li = 0;
    while (li < less.size() && !heavy.empty()) {
      const int l = less[li++];
      const int r = heavy.back();
      heavy.pop_back();
      alias_[l] = r;
      prob_[r] -= 1.0 - prob_[l];  // r закрывает недолив l
      if (prob_[r] < 1.0) {
        // r теперь тоже «лёгкий» (с точностью до ошибки округления) —
        // в конец очереди less, обработаемся позже.
        less.push_back(r);
      } else {
        // r всё ещё «тяжёлый» — вернули в heavy.
        heavy.push_back(r);
      }
    }
    // Остаток (если есть из-за ошибок округления) — prob_ ~= 1,
    // alias_ на себя: корзина всегда отдаёт своё значение.
  }

  std::vector<int> values_;
  std::vector<double> prob_;   // P(«корзина i -> values_[i]»), ~= 1
  std::vector<int> alias_;     // запасное значение корзины i
};
