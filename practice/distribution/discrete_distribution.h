#pragma once

#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>
#include <algorithm>

// Сложность: конструктор O(n), Sample() O(log n) (lower_bound по CDF;
// хеш-таблица неприменима — запрос порядковый, а не по ключу).
// Копирование и move — дефолтные: переезжают values_ и cdf_.
// После std::move источник валиден, но cdf_ пуст;
// Sample() на нём бросает std::logic_error.
class DiscreteDistribution {
public:
    DiscreteDistribution(std::vector<int> values, std::vector<double> weights)
        : values_(std::move(values)), cdf_(weights.size()) {

            if (values.empty() || weights.empty()) {
                throw std::invalid_argument("values и weights не должны быть пустыми");
            }

            if (values.size() != weights.size()) {
                throw std::invalid_argument("Длины values и weights должны совпадать");
            }

            for (size_t i = 0; i < weights.size(); ++i) {
                if (weights[i] <= 0)
                {
                    throw std::invalid_argument("все элементы weights должны быть строго положительными");
                }
            }

            cdf_[0] = weights[0];
            for (size_t i = 1; i < weights.size(); ++i) {cdf_[i] = cdf_[i - 1] + weights[i];}
        }

    int Sample(std::mt19937_64& rng) const {
        if (cdf_.empty()) {
            throw std::logic_error("Sample() на moved-from / пустом объекте");
        }
        double r = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
        double target = r * cdf_.back();
        auto it = std::lower_bound(cdf_.begin(), cdf_.end(), target);
        if (it == cdf_.end()) return values_.back();
        // индекс = итератор - итератор
        return values_[it - cdf_.begin()];
    }

private:
    std::vector<int> values_;
    std::vector<double> cdf_;
};
