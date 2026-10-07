#pragma once

#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

// Legacy-класс генератора дискретного распределения.
// Пришёл к нам в кодбазе в 2019 году, работоспособен, но в ревью
// на него были замечания. Ваша задача — развить его по ТЗ в main.cpp.
class DiscreteDistribution {
public:
    // values[i] рождается с вероятностью, пропорциональной weights[i].
    DiscreteDistribution(std::vector<int> values, std::vector<double> weights)
        : values_(std::move(values)), weights_(std::move(weights)),
          rng_(std::random_device{}()) {}

    // Возвращает следующее значение согласно распределению.
    int Sample() {
        // Текущая реализация: линейный проход.
        const double r =
            std::uniform_real_distribution<double>(0.0, 1.0)(rng_);
        double total = 0.0;
        for (const double w : weights_) {
            total += w;
        }
        const double target = r * total;

        double acc = 0.0;
        for (std::size_t i = 0; i < values_.size(); ++i) {
            acc += weights_[i];
            if (target < acc) {
                return values_[i];
            }
        }
        // Страховка от накопления ошибок двойной точности на хвосте.
        return values_.back();
    }

private:
    std::vector<int> values_;
    std::vector<double> weights_;
    std::mt19937_64 rng_;
};
