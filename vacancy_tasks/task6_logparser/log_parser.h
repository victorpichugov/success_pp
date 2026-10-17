#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

// Событие объекта из лога.
struct ObjectEvent {
  long long id = 0;
  double t = 0.0;
  double x = 0.0;
  double y = 0.0;
};

// Результат парсинга: валидные события (порядок строк) + счётчик
// пропущенных (повреждённых / пустых) строк.
struct ParseResult {
  std::vector<ObjectEvent> events;
  std::size_t skipped = 0;
};

// Legacy: sscanf с ЗАФИКСИРОВАННЫМ порядком полей.
//
// Замечания ревьюера:
//   #1 работает только на каноничном порядке "t=... id=... x=... y=...";
//   #2 дополнительные поля (type=car) ломают разбор;
//   #3 не проверяет «разобрано ЦЕЛИКОМ» ("1.5.5" -> 1.5 + хвост);
//   #4 "nan"/"inf" в числах — пропускаются молча (isfinite не смотрим);
//   #5 пустая строка засчитывается как skipped — случайно, а не по ТЗ.
//
// ТЗ (README.md): порядок любой, доп. поля игнорировать, значения —
// целиком и конечные, повреждённые строки -> skipped (без исключений).
ParseResult ParseLog(const std::vector<std::string>& lines) {
  ParseResult res;
  res.events.reserve(lines.size());
  for (const std::string& line : lines) {
    long long id = 0;
    double t = 0.0;
    double x = 0.0;
    double y = 0.0;
    const int matched =
        std::sscanf(line.c_str(), "t=%lf id=%lld x=%lf y=%lf", &t, &id, &x, &y);
    if (matched == 4) {
      res.events.push_back(ObjectEvent{id, t, x, y});
    } else {
      ++res.skipped;
    }
  }
  return res;
}
