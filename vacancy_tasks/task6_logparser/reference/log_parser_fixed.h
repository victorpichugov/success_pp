#pragma once

#include <cmath>
#include <cstddef>
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

// Разбор «живого» формата: key=value через пробелы, порядок ЛЮБОЙ,
// неизвестные ключи игнорируются, значения — целиком и конечные.
//
// Time: O(сумма длин строк). Space: O(кол-во валидных строк).
// Повреждённые строки не бросают исключения наружу -> skipped.
ParseResult ParseLog(const std::vector<std::string>& lines) {
  ParseResult res;
  res.events.reserve(lines.size());

  for (const std::string& line : lines) {
    if (line.empty()) {
      ++res.skipped;
      continue;
    }
    ObjectEvent ev;
    bool has_id = false;
    bool has_t = false;
    bool has_x = false;
    bool has_y = false;
    bool ok = true;
    try {
      std::size_t pos = 0;
      while (pos < line.size() && ok) {
        if (line[pos] == ' ') {
          ++pos;
          continue;
        }
        const auto eq = line.find('=', pos);
        if (eq == std::string::npos) {
          ok = false;  // нет '=' -> мусор
          break;
        }
        const std::string key = line.substr(pos, eq - pos);
        const std::size_t vpos = eq + 1;
        std::size_t vend = line.find(' ', vpos);
        if (vend == std::string::npos) {
          vend = line.size();
        }
        const std::string val = line.substr(vpos, vend - vpos);
        if (val.empty()) {
          ok = false;  // пустое значение
          break;
        }
        if (key == "id") {
          std::size_t consumed = 0;
          ev.id = std::stoll(val, &consumed);
          if (consumed != val.size()) {  // "7abc", "1.5" -> повреждено
            ok = false;
            break;
          }
          has_id = true;
        } else if (key == "t" || key == "x" || key == "y") {
          std::size_t consumed = 0;
          const double v = std::stod(val, &consumed);
          if (consumed != val.size() || !std::isfinite(v)) {
            ok = false;  // "1.5.5", "nan", "inf"
            break;
          }
          if (key == "t") {
            ev.t = v;
            has_t = true;
          } else if (key == "x") {
            ev.x = v;
            has_x = true;
          } else {
            ev.y = v;
            has_y = true;
          }
        }
        // Неизвестные ключи — игнорируем (format evolves between modules)
        pos = vend;
      }
    } catch (const std::exception&) {
      // stoll/stod: invalid_argument (пусто/не число) или
      // out_of_range (переполнение long long / double) -> строка бита
      ok = false;
    }
    if (ok && has_id && has_t && has_x && has_y) {
      res.events.push_back(ev);
    } else {
      ++res.skipped;
    }
  }
  return res;
}
