#pragma once

#include <cstddef>
#include <string>
#include <utility>

// Legacy (2021). Работает, но на ревью:
//   #1 O(n^2) по кол-ву смайликов: на каждый найденный смайлик —
//      substr(0, pos) + substr(end) — две копии ОСТАТКА строки.
//      50 000 смайликов в строке 100 000 символов ~ 2.5*10^9 копий байт.
//   #2 Выход копится в cur, вход сразу копируется в cur (лишняя копия).
//
// ТЗ — в README.md: const&-вход, один проход, O(n) время, O(n) память.

struct SmileysStats {
  std::size_t happy = 0;  // удалено смайликов ":-" + ")"...
  std::size_t sad = 0;    // удалено смайликов ":-" + "("...
};

std::pair<std::string, SmileysStats> CleanLine(const std::string& line) {
  std::string cur = line;  // копия #1
  SmileysStats stats;
  std::size_t pos = 0;
  while (pos < cur.size()) {
    if (pos + 2 < cur.size() && cur[pos] == ':' && cur[pos + 1] == '-' &&
        (cur[pos + 2] == ')' || cur[pos + 2] == '(')) {
      const char paren = cur[pos + 2];
      std::size_t end = pos + 3;
      while (end < cur.size() && cur[end] == paren) {
        ++end;
      }
      if (paren == ')') {
        ++stats.happy;
      } else {
        ++stats.sad;
      }
      cur = cur.substr(0, pos) + cur.substr(end);  // копия остатка O(n)
      // pos не сдвигаем: на этой позиции теперь следующее слово
    } else {
      ++pos;
    }
  }
  return {cur, stats};
}
