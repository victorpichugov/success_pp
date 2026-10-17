#pragma once

#include <cstddef>
#include <string>
#include <utility>

// Чистка смайликов из const-строки + статистика удалённых.
//
// Грамматика (см. README.md): ":-" + непустой пропуск ОДИНАКОВЫХ
// скобок (все ")" или все "("); удаляется максимальный пропуск
// вместе с ":-". Всё остальное — как есть.
//
// Time: O(n) — один проход, read только движется вперёд.
// Space: O(n) — выход (единственное выделение; reserve снимает
// реаллокации).
//
// В-place (как в practice/smiley_remover) здесь НЕЛЬЗЯ: вход const,
// он принадлежит другому модулю/потоку. «Два указателя» остаются в
// духе: read-указатель по входу, write — через push_back в выход.

struct SmileysStats {
  std::size_t happy = 0;  // удалено смайликов ":-" + ")"...
  std::size_t sad = 0;    // удалено смайликов ":-" + "("...
};

std::pair<std::string, SmileysStats> CleanLine(const std::string& line) {
  std::string out;
  out.reserve(line.size());  // выход не длиннее входа — без реаллокаций
  SmileysStats stats;

  const std::size_t n = line.size();
  std::size_t read = 0;
  while (read < n) {
    // Смайлик начинается только если есть место на ":-X" (read + 2).
    if (read + 2 < n && line[read] == ':' && line[read + 1] == '-' &&
        (line[read + 2] == ')' || line[read + 2] == '(')) {
      const char paren = line[read + 2];
      if (paren == ')') {
        ++stats.happy;
      } else {
        ++stats.sad;
      }
      read += 3;
      while (read < n && line[read] == paren) {
        ++read;  // максимальный пропуск одинаковых скобок
      }
    } else {
      out.push_back(line[read]);
      ++read;
    }
  }
  return {std::move(out), stats};
}
