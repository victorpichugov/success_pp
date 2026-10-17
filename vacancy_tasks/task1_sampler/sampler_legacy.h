#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

// CdfSampler — «текущий» прод-подход: CDF + lower_bound.
// Конструктор O(n), Sample O(log n) + рандомный доступ по CDF-вектору.
// Для ориентира: сравнить по перфу (тест f) и по вырожденному кейсу
// огромных весов (тест g). Валидация неполная — legacy-составляющая.
class CdfSampler {
 public:
  CdfSampler(std::vector<int> values, std::vector<double> weights)
      : values_(std::move(values)), cdf_(weights.size()) {
    if (values_.empty() || weights.empty() ||
        values_.size() != weights.size()) {
      throw std::invalid_argument("bad arguments");
    }
    cdf_[0] = weights[0];
    for (std::size_t i = 1; i < weights.size(); ++i) {
      cdf_[i] = cdf_[i - 1] + weights[i];
    }
  }

  int Sample(std::mt19937_64& rng) const {
    const double t = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    const double target = t * cdf_.back();
    const auto it = std::lower_bound(cdf_.begin(), cdf_.end(), target);
    if (it == cdf_.end()) {
      return values_.back();
    }
    return values_[it - cdf_.begin()];
  }

 private:
  std::vector<int> values_;
  std::vector<double> cdf_;
};
