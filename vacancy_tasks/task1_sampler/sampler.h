#pragma once

#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

// AliasedSampler — дискретное распределение с O(1)-сэмплированием
// (Walker's Alias Method). Полное ТЗ — в README.md.
//
// Контракт:
//   - конструктор бросает std::invalid_argument на невалидных входных
//     данных (пустые, разнобой размеров, вес <= 0 / NaN / Inf,
//     неконечная сумма);
//   - после std::move исходник валиден, но пуст; Sample() на нём
//     бросает std::logic_error (это защита, а не UB).
//
// Сложности (заполни после реализации):
//   конструктор: O(?), память O(?);
//   Sample(): O(?) в среднем, выделений в горячем пути: да/нет.
class AliasedSampler {
 public:
  // TODO: O(n), память O(n).
  AliasedSampler(std::vector<int> values, std::vector<double> weights)
      : values_(std::move(values)) {
    (void)weights;
    throw std::logic_error("TODO: реализовать конструктор");
  }

  // TODO: O(1) в среднем, без выделений.
  int Sample(std::mt19937_64& rng) const {
    (void)rng;
    throw std::logic_error("TODO: реализовать Sample");
  }

  std::size_t Size() const { return values_.size(); }

 private:
 // Подсказка: три вектора размера n:
 //   values_ — вход (уже перемещён в конструкторе);
 //   prob_[i] — вероятность того, что «корзина i» даст values_[i]
 //              (0 <= prob_ <= 1, сумма по всем корзинам = n);
 //   alias_[i] — запасное значение корзины i.
 std::vector<int> values_;
};
