#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

// Два указателя по отсортированным массивам.
//
// Идея: максимум достигается сразу ПОСЛЕ какого-то въезда
// (число присутствующих растёт только на въездах). Проходим въезды
// по одному указателю i; второй указатель left считает, сколько
// машин уже выехали к моменту enter[i] (exit[left] <= enter[i]).
// left только движется вперёд (обе последовательности неубывающие)
// -> суммарно O(n).
//
// Time: O(n). Space: O(1) доп. (входы не трогаем).
//
// Конвенция: присутствие в t, если enter <= t < exit.
// Момент enter[i] считаем ПОСЛЕ обработки всех выездов exit <= enter[i].
// Несколько машин с одинаковым enter[i]: после i-го (последнего из
// равных) счёт равен истинному числу присутствующих в момент t.
long long MaxConcurrent(const std::vector<long long>& enter,
                        const std::vector<long long>& exit) {
  long long result = 0;
  std::size_t left = 0;  // сколько машин уже выехало (exit <= текущего t)
  for (std::size_t i = 0; i < enter.size(); ++i) {
    while (left < exit.size() && exit[left] <= enter[i]) {
      ++left;
    }
    // Присутствуют: все, кто въехал до и включая i (i + 1),
    // минус все, кто уже выехал (left).
    result = std::max(result, static_cast<long long>(i + 1 - left));
  }
  return result;
}
