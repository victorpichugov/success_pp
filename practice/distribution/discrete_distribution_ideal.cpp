// DiscreteDistribution — генератор дискретного распределения
// и комплект проверок (пункты ТЗ a-e).
//
// values[i] рождается с вероятностью w_i / (w_0 + ... + w_{n-1}).
//
// Проектные решения:
//  - CDF (кумулятивные веса) строится в конструкторе: O(n) один раз,
//    каждый Sample() — O(log n) через std::lower_bound. Хеш-таблица
//    не подходит: запрос порядковый ("в какой сегмент попала точка"),
//    а у хеш-таблицы понятие порядка отсутствует.
//  - RNG передается в Sample() вызывающим по ссылке: сохраняется
//    const-корректность, объект можно шарить между потоками
//    (у каждого свой rng), тесты детерминированы
//    (один seed — одна последовательность). Так же устроен
//    std::discrete_distribution.
//  - после построения CDF оригинальные веса не нужны (восстанавливаются
//    разностями), поэтому один вектор в памяти, а не два.
//
// Сложность:
//  - конструктор: O(n) по времени, O(n) по памяти;
//  - Sample(): O(log n);
//  - k сэмплов: O(n + k log n) (legacy-версия давала O(n * k)).
//
// Контракт:
//  - класс копируем и перемещаем, операции копирования/перемещения
//    неявные;
//  - moved-from объект валиден, но пуст: Sample() на нем бросает
//    std::logic_error (защита вместо UB);
//  - CDF хранится в double: вес, различающийся с суммой остальных
//    более чем на ~10^15, теряет точность и может получить
//    нулевую вероятность; при таких перекосах нормализуйте веса.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

class DiscreteDistribution {
 public:
  // Precondition: values непуст, размеры совпадают, все веса > 0.
  // В противном случае бросает std::invalid_argument.
  DiscreteDistribution(std::vector<int> values, std::vector<double> weights);

  // Возвращает следующее значение согласно распределению.
  // rng мутируется: движок сдвигает свое состояние.
  int Sample(std::mt19937_64& rng) const;

 private:
  std::vector<int> values_;
  std::vector<double> cdf_;  // cdf_[i] - правая граница i-го сегмента
};

DiscreteDistribution::DiscreteDistribution(std::vector<int> values,
                                           std::vector<double> weights)
    : values_(std::move(values)) {
  if (values.empty()) {
    throw std::invalid_argument("values must be non-empty");
  }
  if (values.size() != weights.size()) {
    throw std::invalid_argument(
        "values.size() and weights.size() must be equal");
  }
  // Валидируем весь массив до мутации состояния: иначе исключение
  // на середине оставило бы за собой частично заполненный CDF.
  for (std::size_t i = 0; i < weights.size(); ++i) {
    if (weights[i] <= 0.0) {
      throw std::invalid_argument(
          "all weights must be strictly positive");
    }
  }

  cdf_.resize(weights.size());
  cdf_[0] = weights[0];
  for (std::size_t i = 1; i < cdf_.size(); ++i) {
    cdf_[i] = cdf_[i - 1] + weights[i];
  }
}

int DiscreteDistribution::Sample(std::mt19937_64& rng) const {
  if (cdf_.empty()) {
    throw std::logic_error(
        "Sample() called on a moved-from (empty) object");
  }
  const double r = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
  const double target = r * cdf_.back();
  const auto it = std::lower_bound(cdf_.begin(), cdf_.end(), target);
  // В точной арифметике target < cdf_.back(), но округление
  // произведения r * total может дойти до конца CDF - страховка.
  if (it == cdf_.end()) {
    return values_.back();
  }
  return values_[static_cast<std::size_t>(it - cdf_.begin())];
}

namespace {

void PrintFailure(const char* test, const std::string& detail) {
  std::cerr << "FAIL [" << test << "]: " << detail << '\n';
}

// a) Детерминизм: одинаковый seed => одинаковая последовательность,
//    разный seed => другая.
int TestDeterminism() {
  const int kSamples = 1000;
  std::mt19937_64 seed7a{7};
  std::mt19937_64 seed7b{7};
  std::mt19937_64 seed8{8};
  const DiscreteDistribution d1({1, 2, 3}, {1.0, 3.0, 5.0});
  const DiscreteDistribution d2({1, 2, 3}, {1.0, 3.0, 5.0});

  bool sameSeedsAgree = true;
  bool diffSeedDiffers = false;
  for (int i = 0; i < kSamples; ++i) {
    const int a = d1.Sample(seed7a);
    const int b = d2.Sample(seed7b);
    const int c = d2.Sample(seed8);
    if (a != b) {
      sameSeedsAgree = false;
    }
    if (a != c) {
      diffSeedDiffers = true;
    }
  }

  int failures = 0;
  if (!sameSeedsAgree) {
    PrintFailure("a", "одинаковые seed'ы дали разные последовательности");
    ++failures;
  }
  if (!diffSeedDiffers) {
    PrintFailure("a", "разные seed'ы дали одинаковые последовательности");
    ++failures;
  }
  return failures;
}

// b) Частоты: weights {1, 3, 5} => p = {1/9, 3/9, 5/9},
//    отклонение каждой частоты <= 2% при 1e6 сэмплах.
int TestFrequencies() {
  const int kSamples = 1000000;
  const double kTolerance = 0.02;
  std::mt19937_64 rng{42};
  const DiscreteDistribution d({1, 2, 3}, {1.0, 3.0, 5.0});
  const double expected[3] = {1.0 / 9.0, 3.0 / 9.0, 5.0 / 9.0};

  std::vector<long long> counts(3, 0);
  for (int i = 0; i < kSamples; ++i) {
    const int x = d.Sample(rng);
    // values = {1, 2, 3}: индекс в counts равен x - 1.
    // Сбой этой проверки - баг в Sample(), останавливаемся сразу.
    if (x < 1 || x > 3) {
      PrintFailure("b", "сэмпл вернул значение вне диапазона");
      return 1;
    }
    ++counts[x - 1];
  }

  int failures = 0;
  for (int i = 0; i < 3; ++i) {
    const double freq = static_cast<double>(counts[i]) / kSamples;
    if (std::abs(freq - expected[i]) > kTolerance) {
      PrintFailure("b", "freq[" + std::to_string(i) + "] = " +
                           std::to_string(freq) + ", ожидалось ~" +
                           std::to_string(expected[i]));
      ++failures;
    }
  }
  return failures;
}

// c) Валидация: каждая неверная комбинация бросает
//    std::invalid_argument.
int TestValidation() {
  int failures = 0;
  auto checkThrows = [&failures](const char* name, auto construct) {
    bool thrown = false;
    try {
      construct();
    } catch (const std::invalid_argument& e) {
      std::cout << "  [" << name << "] бросило: " << e.what() << '\n';
      thrown = true;
    }
    if (!thrown) {
      PrintFailure("c", std::string(name) + ": исключение не бросилось");
      ++failures;
    }
  };

  checkThrows("c1 пустые", [] {
    DiscreteDistribution d({}, {1.0});
  });
  checkThrows("c2 разные размеры", [] {
    DiscreteDistribution d({1, 2}, {1.0});
  });
  checkThrows("c3 нулевой вес", [] {
    DiscreteDistribution d({1, 2}, {0.0, 5.0});
  });
  // c4 - регрессия на баг "цикл валидации с i = 1":
  // нехороший вес стоит ПЕРВЫМ.
  checkThrows("c4 отрицательный вес", [] {
    DiscreteDistribution d({1, 2}, {-1.0, 5.0});
  });
  return failures;
}

// d) Граничные случаи.
int TestEdgeCases() {
  int failures = 0;

  // d1: одно значение - возвращается всегда.
  {
    std::mt19937_64 rng{1};
    const DiscreteDistribution d({42}, {7.0});
    for (int i = 0; i < 1000; ++i) {
      if (d.Sample(rng) != 42) {
        PrintFailure("d1", "единственное значение не всегда");
        ++failures;
        break;
      }
    }
  }

  // d2: сильный перекос весов - ограничение точности double.
  // 1e16 + 1.0 == 1e16 (у double ~15-16 значащих цифр), поэтому
  // cdf = [1e16, 1e16]: второй сегмент имеет нулевую длину и
  // значение 2 имеет РОВНО нулевую вероятность. Это не баг
  // алгоритма, а задокументированное ограничение представления.
  {
    std::mt19937_64 rng{3};
    const DiscreteDistribution d({1, 2}, {1e16, 1.0});
    long long cnt2 = 0;
    const int kSamples = 100000;
    for (int i = 0; i < kSamples; ++i) {
      if (d.Sample(rng) == 2) {
        ++cnt2;
      }
    }
    if (cnt2 != 0) {
      PrintFailure("d2", "значение 2 выдано при нулевом сегменте");
      ++failures;
    }
  }
  return failures;
}

// e) Move-семантика: moved-to работает, moved-from бросает
//    std::logic_error (контролируемо, а не UB).
int TestMoveSemantics() {
  int failures = 0;
  DiscreteDistribution src({1, 2, 3}, {1.0, 3.0, 5.0});
  DiscreteDistribution dst = std::move(src);

  std::mt19937_64 rng{5};

  const int x = dst.Sample(rng);
  if (x < 1 || x > 3) {
    PrintFailure("e", "moved-to объект сломан");
    ++failures;
  }

  bool thrown = false;
  try {
    src.Sample(rng);
  } catch (const std::logic_error&) {
    thrown = true;
  }
  if (!thrown) {
    PrintFailure("e", "moved-from не бросил std::logic_error");
    ++failures;
  }
  return failures;
}

}  // namespace

int main() {
  int failures = 0;
  failures += TestDeterminism();
  failures += TestFrequencies();
  failures += TestValidation();
  failures += TestEdgeCases();
  failures += TestMoveSemantics();

  if (failures == 0) {
    std::cout << "ALL TESTS PASSED\n";
  } else {
    std::cerr << "FAILED: " << failures << " check(s)\n";
  }
  return failures;
}
